#!/usr/bin/env python3
"""
Test Extraction Tool for Luka Testing Framework

Extracts test resources (config, www files, cgi scripts) and generates
curl/telnet command files for manual testing without running webserv.

Usage:
    python luka/extract_test.py test_file_name

Example:
    python luka/extract_test.py test_basic_request
"""

import os
import sys
import ast
import re
import shutil
import inspect
import importlib.util
import tempfile
from typing import List, Dict, Optional, Tuple, Any

# Add project root to sys.path
WEBSERV_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, WEBSERV_PATH)

from luka.base_test import BaseTest

TESTS_DIR = os.path.join(WEBSERV_PATH, "luka", "tests")


class HTTPRequest:
    """Represents an HTTP request extracted from test code."""

    def __init__(
        self,
        method: str,
        path: str,
        headers: Optional[Dict] = None,
        data: Optional[str] = None,
        http_version: str = "1.0",
    ):
        self.method = method.upper()
        self.path = path
        self.headers = headers or {}
        self.data = data
        self.http_version = http_version
        self.concurrent = False  # Whether this is part of concurrent requests
        self.concurrent_group: Optional[int] = (
            None  # ID to group concurrent requests together
        )

    def __repr__(self):
        return f"HTTPRequest({self.method} {self.path})"


class HTTPRequestParser:
    """Parses HTTP requests from test's run() method using AST."""

    def __init__(self, test_instance: BaseTest):
        self.test_instance = test_instance
        self.requests: List[HTTPRequest] = []
        self.variables: Dict[str, str] = {}  # Track variable assignments

    def parse(self) -> List[HTTPRequest]:
        """Parse all HTTP requests from the run() method."""
        try:
            import textwrap

            source = inspect.getsource(self.test_instance.run)
            # Dedent the source code to remove indentation
            source = textwrap.dedent(source)
            tree = ast.parse(source)

            # First pass: collect variable assignments
            for node in ast.walk(tree):
                if isinstance(node, ast.Assign):
                    for target in node.targets:
                        if isinstance(target, ast.Name):
                            var_name = target.id
                            var_value = self._extract_arg_value(node.value)
                            self.variables[var_name] = var_value

            # Second pass: extract HTTP requests
            for node in ast.walk(tree):
                if isinstance(node, ast.Call):
                    if isinstance(node.func, ast.Attribute):
                        method_name = node.func.attr
                        if method_name in ["get", "post", "put", "delete", "head"]:
                            request = self._extract_request(method_name, node)
                            if request:
                                self.requests.append(request)
                        # Detect concurrent_get calls
                        elif method_name == "concurrent_get":
                            concurrent_requests = self._extract_concurrent_get(node)
                            self.requests.extend(concurrent_requests)

            return self.requests
        except Exception as e:
            print(f"Warning: Could not parse HTTP requests: {e}")
            return []

    def _extract_request(self, method: str, node: ast.Call) -> Optional[HTTPRequest]:
        """Extract request details from AST node."""
        try:
            # Extract path (first positional argument)
            path = self._extract_arg_value(node.args[0]) if node.args else "/"

            # Extract keyword arguments
            headers = None
            data = None
            http_version = "1.0"

            for keyword in node.keywords:
                if keyword.arg == "headers":
                    headers = self._extract_dict_value(keyword.value)
                elif keyword.arg == "data":
                    data = self._extract_arg_value(keyword.value)
                elif keyword.arg == "http_version":
                    http_version = self._extract_arg_value(keyword.value)

            return HTTPRequest(method, path, headers, data, http_version)
        except Exception as e:
            print(f"Warning: Could not extract request details: {e}")
            return None

    def _extract_arg_value(self, node) -> str:
        """Extract value from AST node."""
        if isinstance(node, ast.Constant):
            return str(node.value)
        elif isinstance(node, ast.JoinedStr):  # f-string
            return "{dynamic_value}"
        elif isinstance(node, ast.Name):
            # Try to resolve variable name
            var_name = node.id
            if var_name in self.variables:
                return self.variables[var_name]
            return f"{{{node.id}}}"
        else:
            return "{dynamic_value}"

    def _extract_dict_value(self, node) -> Optional[Dict[str, str]]:
        """Extract dictionary from AST node."""
        if isinstance(node, ast.Dict):
            result = {}
            for key, value in zip(node.keys, node.values):
                key_str = self._extract_arg_value(key)
                value_str = self._extract_arg_value(value)
                result[key_str] = value_str
            return result
        return None

    def _extract_concurrent_get(self, node: ast.Call) -> List[HTTPRequest]:
        """Extract concurrent GET requests from concurrent_get() call."""
        requests = []
        try:
            # concurrent_get() first argument is a list of URLs
            if node.args and isinstance(node.args[0], ast.List):
                urls_list = node.args[0]
                for idx, url_node in enumerate(urls_list.elts):
                    url = self._extract_arg_value(url_node)
                    # Extract headers and http_version from keywords if present
                    headers = None
                    http_version = "1.0"
                    for keyword in node.keywords:
                        if keyword.arg == "headers":
                            headers = self._extract_dict_value(keyword.value)
                        elif keyword.arg == "http_version":
                            http_version = self._extract_arg_value(keyword.value)

                    # Mark as concurrent request in the path
                    request = HTTPRequest("GET", url, headers, None, http_version)
                    request.concurrent = True
                    request.concurrent_group = id(
                        node
                    )  # Group ID for concurrent requests
                    requests.append(request)
        except Exception as e:
            print(f"Warning: Could not extract concurrent_get requests: {e}")

        return requests


class CommandGenerator:
    """Generates curl and telnet command files."""

    def __init__(
        self, test_name: str, description: str, port: int, requests: List[HTTPRequest]
    ):
        self.test_name = test_name
        self.description = description
        self.port = port
        self.requests = requests

    def generate_curl_script(self) -> str:
        """Generate executable bash script with curl commands."""
        lines = [
            "#!/bin/bash",
            f"# Test: {self.test_name}",
            f"# Description: {self.description}",
            f"# Port: {self.port}",
            "",
            f'echo "=== Test: {self.test_name} ==="',
            f'echo "Start webserv with: ./build/webserv {self.test_name}/{self.test_name}.conf"',
            'echo ""',
            "",
        ]

        for idx, request in enumerate(self.requests, 1):
            lines.append(f"# Request {idx}: {request.method} {request.path}")
            lines.append(f'echo ">>> Request {idx}: {request.method} {request.path}"')
            lines.append(self._generate_curl_command(request))
            lines.append('echo ""')
            lines.append('echo "---"')
            lines.append('echo ""')

        return "\n".join(lines)

    def _generate_curl_command(self, request: HTTPRequest) -> str:
        """Generate a single curl command."""
        cmd = f"curl -i -X {request.method} http://localhost:{self.port}{request.path}"

        # Add HTTP version flag
        if request.http_version == "1.0":
            cmd += " --http1.0"

        # Add headers
        for key, value in request.headers.items():
            cmd += f' \\\n  -H "{key}: {value}"'

        # Always add Connection: close
        cmd += ' \\\n  -H "Connection: close"'

        # Add data for POST/PUT requests
        if request.data and request.method in ["POST", "PUT"]:
            # Escape double quotes in data
            escaped_data = request.data.replace('"', '\\"')
            cmd += f' \\\n  -d "{escaped_data}"'

        return cmd

    def generate_telnet_file(self) -> str:
        """Generate telnet commands text file."""
        lines = [
            f"# Test: {self.test_name}",
            f"# Description: {self.description}",
            f"# Port: {self.port}",
            "#",
            "# Usage: telnet localhost {port}".replace("{port}", str(self.port)),
            "# Then copy/paste each request below (including the blank line at the end)",
            "",
        ]

        for idx, request in enumerate(self.requests, 1):
            lines.append("# " + "=" * 40)
            lines.append(f"# Request {idx}: {request.method} {request.path}")
            lines.append("# " + "=" * 40)
            lines.append(self._generate_telnet_request(request))
            lines.append("")

        return "\n".join(lines)

    def _generate_telnet_request(self, request: HTTPRequest) -> str:
        """Generate a single telnet request."""
        lines = []

        # Request line
        lines.append(f"{request.method} {request.path} HTTP/{request.http_version}")
        lines.append(f"Host: localhost:{self.port}")

        # Calculate content length for POST/PUT
        content_length = 0
        if request.data and request.method in ["POST", "PUT"]:
            content_length = len(request.data)
            lines.append(f"Content-Length: {content_length}")

        # Add custom headers
        for key, value in request.headers.items():
            lines.append(f"{key}: {value}")

        # Always add Connection: close
        lines.append("Connection: close")

        # Empty line to end headers
        lines.append("")

        # Add body if present
        if request.data and request.method in ["POST", "PUT"]:
            lines.append(request.data)

        return "\n".join(lines)

    def generate_parallel_script(self) -> str:
        """
        Generate executable bash script with parallel execution for concurrent requests.
        Uses background jobs (&) and wait for concurrent groups.
        """
        lines = [
            "#!/bin/bash",
            f"# Test: {self.test_name}",
            f"# Description: {self.description}",
            f"# Port: {self.port}",
            "#",
            "# This script executes requests in parallel where appropriate",
            "",
            f'echo "=== Test: {self.test_name} (Parallel Execution) ==="',
            f'echo "Start webserv with: ./build/webserv {self.test_name}/{self.test_name}.conf"',
            'echo ""',
            "",
        ]

        # Group requests by concurrent_group
        from collections import defaultdict

        concurrent_groups = defaultdict(list)
        sequential_requests = []

        for idx, request in enumerate(self.requests):
            if request.concurrent and request.concurrent_group is not None:
                concurrent_groups[request.concurrent_group].append((idx, request))
            else:
                sequential_requests.append((idx, request))

        # Process all requests in order, but handle concurrent groups specially
        processed_groups = set()

        for idx, request in enumerate(self.requests):
            # If this is part of a concurrent group and we haven't processed it yet
            if request.concurrent and request.concurrent_group is not None:
                if request.concurrent_group not in processed_groups:
                    processed_groups.add(request.concurrent_group)
                    group_requests = concurrent_groups[request.concurrent_group]

                    lines.append(
                        f"# Concurrent group: {len(group_requests)} parallel requests"
                    )
                    lines.append('echo ">>> Starting concurrent requests..."')

                    # Launch all requests in the group as background jobs
                    for req_idx, req in group_requests:
                        lines.append(
                            f"# Background request {req_idx + 1}: {req.method} {req.path}"
                        )
                        lines.append(self._generate_curl_command(req) + " &")

                    # Wait for all background jobs
                    lines.append("")
                    lines.append(
                        'echo ">>> Waiting for concurrent requests to complete..."'
                    )
                    lines.append("wait")
                    lines.append('echo ">>> Concurrent requests completed"')
                    lines.append('echo ""')
                    lines.append('echo "---"')
                    lines.append('echo ""')
                    lines.append("")

            # Handle sequential requests
            elif not request.concurrent:
                lines.append(
                    f"# Sequential request {idx + 1}: {request.method} {request.path}"
                )
                lines.append(
                    f'echo ">>> Request {idx + 1}: {request.method} {request.path}"'
                )
                lines.append(self._generate_curl_command(request))
                lines.append('echo ""')
                lines.append('echo "---"')
                lines.append('echo ""')

        return "\n".join(lines)


class TestExtractor:
    """Main extractor class that orchestrates the extraction process."""

    def __init__(self, test_file_name: str):
        self.test_file_name = test_file_name
        self.test_class = None
        self.test_instance = None
        self.output_dir = None
        self.temp_dir = None

    def extract(self):
        """Main extraction workflow."""
        try:
            # Step 1: Load test class
            print(f"Loading test: {self.test_file_name}")
            self.test_class = self._load_test_class()
            if not self.test_class:
                return False

            # Step 2: Create test instance
            self.test_instance = self.test_class()
            test_name = self.test_instance.get_name()
            test_description = self.test_instance.get_description()

            print(f"Extracting test: {test_name}")
            print(f"Description: {test_description}")
            print()

            # Step 3: Setup temporary environment
            self.temp_dir = tempfile.mkdtemp(prefix="luka_extract_")
            self.test_instance.initialize_test_env(self.temp_dir)

            # Step 4: Run test setup to generate files
            print("Running test setup...")
            self.test_instance.setup()

            if not self.test_instance.conf_path or not os.path.exists(
                self.test_instance.conf_path
            ):
                print("Error: Test setup did not create a configuration file.")
                return False

            # Step 5: Create output directory
            self.output_dir = os.path.abspath(test_name)
            print(f"Creating directory: {self.output_dir}/")
            os.makedirs(self.output_dir, exist_ok=True)

            # Step 6: Extract port from config
            port = self._extract_port()

            # Step 7: Copy all files to output directory
            self._copy_files()

            # Step 8: Parse HTTP requests
            print("Parsing HTTP requests...")
            parser = HTTPRequestParser(self.test_instance)
            requests = parser.parse()

            print(f"Found {len(requests)} HTTP request(s):")
            for req in requests:
                print(f"  - {req.method} {req.path}")
            print()

            # Step 9: Generate command files
            generator = CommandGenerator(test_name, test_description, port, requests)

            curl_script = generator.generate_curl_script()
            curl_path = os.path.join(self.output_dir, ".runme.curl")
            with open(curl_path, "w") as f:
                f.write(curl_script)
            os.chmod(curl_path, 0o755)
            print(f"✓ Generated curl commands: {curl_path}")

            # Generate parallel script
            parallel_script = generator.generate_parallel_script()
            parallel_path = os.path.join(self.output_dir, ".runme.parallel.sh")
            with open(parallel_path, "w") as f:
                f.write(parallel_script)
            os.chmod(parallel_path, 0o755)
            print(f"✓ Generated parallel script: {parallel_path}")

            telnet_file = generator.generate_telnet_file()
            telnet_path = os.path.join(self.output_dir, ".runme.telnet")
            with open(telnet_path, "w") as f:
                f.write(telnet_file)
            print(f"✓ Generated telnet commands: {telnet_path}")
            print()

            # Step 10: Print usage instructions
            self._print_usage_instructions(test_name, port)

            return True

        except Exception as e:
            print(f"Error during extraction: {e}")
            import traceback

            traceback.print_exc()
            return False
        finally:
            # Cleanup temp directory
            if self.temp_dir and os.path.exists(self.temp_dir):
                shutil.rmtree(self.temp_dir, ignore_errors=True)

    def _load_test_class(self):
        """Load test class from test file."""
        # Try with and without .py extension
        test_file = self.test_file_name
        if not test_file.endswith(".py"):
            test_file += ".py"

        # Try with and without test_ prefix
        test_paths = [
            os.path.join(TESTS_DIR, test_file),
            os.path.join(TESTS_DIR, f"test_{test_file}"),
        ]

        test_path = None
        for path in test_paths:
            if os.path.exists(path):
                test_path = path
                break

        if not test_path:
            print(f"Error: Test file not found: {self.test_file_name}")
            print(f"Searched in: {TESTS_DIR}")
            return None

        # Load module
        spec = importlib.util.spec_from_file_location("test_module", test_path)
        test_module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(test_module)

        # Find BaseTest subclasses
        test_classes = []
        for item_name in dir(test_module):
            item = getattr(test_module, item_name)
            if (
                isinstance(item, type)
                and issubclass(item, BaseTest)
                and item is not BaseTest
            ):
                test_classes.append(item)

        if not test_classes:
            print(f"Error: No test classes found in {test_path}")
            return None

        if len(test_classes) > 1:
            print(
                f"Warning: Multiple test classes found, using first one: {test_classes[0].__name__}"
            )

        return test_classes[0]

    def _extract_port(self) -> int:
        """Extract port number from configuration file."""
        try:
            with open(self.test_instance.conf_path, "r") as f:
                conf_content = f.read()

            port_match = re.search(r"listen\s+(\d+);", conf_content)
            if port_match:
                return int(port_match.group(1))
        except Exception as e:
            print(f"Warning: Could not extract port from config: {e}")

        # Default to 8080
        return 8080

    def _copy_files(self):
        """Copy all generated files to output directory."""
        test_name = self.test_instance.get_name()

        # Copy config file and fix paths to be relative
        conf_src = self.test_instance.conf_path
        conf_dst = os.path.join(self.output_dir, f"{test_name}.conf")

        # Read config and replace temp paths with relative paths
        with open(conf_src, "r") as f:
            conf_content = f.read()

        # Replace absolute temp paths with relative paths
        www_dst_abs = os.path.join(self.output_dir, "www")
        cgi_dst_abs = os.path.join(self.output_dir, "cgi-bin")
        uploads_dst_abs = os.path.join(self.output_dir, "uploads")
        logs_dst_abs = os.path.join(self.output_dir, "logs")

        conf_content = conf_content.replace(self.test_instance.www_dir, www_dst_abs)
        conf_content = conf_content.replace(self.test_instance.cgi_dir, cgi_dst_abs)
        conf_content = conf_content.replace(
            self.test_instance.uploads_dir, uploads_dst_abs
        )
        conf_content = conf_content.replace(self.test_instance.logs_dir, logs_dst_abs)

        with open(conf_dst, "w") as f:
            f.write(conf_content)

        print(f"✓ Extracted config: {conf_dst}")

        # Copy www directory
        www_src = self.test_instance.www_dir
        www_dst = os.path.join(self.output_dir, "www")
        if os.path.exists(www_src) and os.listdir(www_src):
            shutil.copytree(www_src, www_dst, dirs_exist_ok=True)
            file_count = sum(len(files) for _, _, files in os.walk(www_dst))
            print(f"✓ Extracted www files: {file_count} file(s)")
        else:
            os.makedirs(www_dst, exist_ok=True)
            print(f"✓ Created www directory: {www_dst}/")

        # Copy cgi-bin directory
        cgi_src = self.test_instance.cgi_dir
        cgi_dst = os.path.join(self.output_dir, "cgi-bin")
        if os.path.exists(cgi_src) and os.listdir(cgi_src):
            shutil.copytree(cgi_src, cgi_dst, dirs_exist_ok=True)
            file_count = sum(len(files) for _, _, files in os.walk(cgi_dst))
            print(f"✓ Extracted cgi-bin files: {file_count} file(s)")
        else:
            os.makedirs(cgi_dst, exist_ok=True)
            print(f"✓ Created cgi-bin directory: {cgi_dst}/")

        # Create empty uploads directory
        uploads_dst = os.path.join(self.output_dir, "uploads")
        os.makedirs(uploads_dst, exist_ok=True)
        print(f"✓ Created upload directory: {uploads_dst}/")

    def _print_usage_instructions(self, test_name: str, port: int):
        """Print usage instructions."""
        print("=" * 60)
        print("To test manually:")
        print(f"1. Start webserv: ./build/webserv {test_name}/{test_name}.conf")
        print(f"2. Run curl tests (sequential): ./{test_name}/.runme.curl")
        print(f"3. Run curl tests (parallel): ./{test_name}/.runme.parallel.sh")
        print(f"4. Or use telnet: telnet localhost {port}")
        print(f"   (Then copy/paste requests from {test_name}/.runme.telnet)")
        print("=" * 60)


def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        print("Usage: python luka/extract_test.py test_file_name")
        print("\nExample:")
        print("  python luka/extract_test.py test_basic_request")
        print("  python luka/extract_test.py test_post")
        sys.exit(1)

    test_file_name = sys.argv[1]
    extractor = TestExtractor(test_file_name)
    success = extractor.extract()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
