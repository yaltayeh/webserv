"""
Bad/invalid configuration file test.
Tests how the server handles various configuration errors and syntax issues.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder
import subprocess
import os
import tempfile


class BadConfigTest(BaseTest):
    def get_name(self) -> str:
        return "bad_config"

    def get_description(self) -> str:
        return "Tests invalid configuration files: syntax errors, invalid directives, bad values."

    def setup(self):
        # Create index page (won't be used but needed for valid config tests)
        self.create_html_file("index.html", HTMLBuilder.index_page("Config Test"))

        # Use a base port for config tests, incrementing for each test
        import random

        self.test_port_base = random.randint(20000, 25000)
        self.test_port_counter = 0

        # Create a minimal config for the runner (won't be used in tests)
        dummy_conf = f"""http {{
    server {{
        listen {self.port};
        root {self.www_dir};
    }}
}}"""
        self.create_conf(dummy_conf)

    def get_test_port(self):
        """Get a unique port for each test to avoid TIME_WAIT issues."""
        port = self.test_port_base + self.test_port_counter
        self.test_port_counter += 1
        return port

    def run(self):
        print("Testing various invalid configuration scenarios...")
        print("Note: Server should reject these configs at startup\n")

        # Get path to webserv binary
        webserv_binary = self.get_webserv_binary()

        # Test 1: Valid configuration (baseline)
        print("Test 1: Valid configuration (baseline)")
        valid_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 1m;
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""
        result = self.test_config(valid_conf, webserv_binary)
        print(
            f"  Result: success={result['success']}, returncode={result.get('returncode', 'N/A')}"
        )
        if not result["success"]:
            print(f"  Stderr: {result.get('stderr', '')}")
            print(f"  Stdout: {result.get('stdout', '')}")
        assert result["success"], "Valid config should be accepted"
        print("✓ Valid config accepted\n")

        # Test 2: Missing semicolon
        print("Test 2: Missing semicolon")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()}
        server_name localhost;
        root {self.www_dir};
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with missing semicolon should be rejected"
        print("✓ Missing semicolon rejected\n")

        # Test 3: Unclosed quote
        print("Test 3: Unclosed quote")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name "localhost;
        root {self.www_dir};
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with unclosed quote should be rejected"
        print("✓ Unclosed quote rejected\n")

        # Test 4: Mismatched braces
        print("Test 4: Mismatched braces")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
    
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with mismatched braces should be rejected"
        print("✓ Mismatched braces rejected\n")

        # Test 5: Invalid port number (too high)
        print("Test 5: Invalid port number (99999)")
        bad_conf = f"""http {{
    server {{
        listen 99999;
        server_name localhost;
        root {self.www_dir};
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with invalid port should be rejected"
        print("✓ Invalid port rejected\n")

        # Test 6: Invalid port number (0)
        print("Test 6: Invalid port number (0)")
        bad_conf = f"""http {{
    server {{
        listen 0;
        server_name localhost;
        root {self.www_dir};
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        # Port 0 should be rejected (invalid port for binding)
        assert not result["success"], "Config with port 0 should be rejected"
        print("✓ Port 0 rejected\n")

        # Test 7: Invalid error code (out of range)
        print("Test 7: Invalid error code (1000)")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        error_page 1000 /error.html;
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with invalid error code should be rejected"
        )
        print("✓ Invalid error code rejected\n")

        # Test 8: Invalid boolean value
        print("Test 8: Invalid boolean value (maybe)")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        
        location /upload {{
            upload_enable maybe;
            upload_store {self.uploads_dir};
        }}
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with invalid boolean should be rejected"
        print("✓ Invalid boolean rejected\n")

        # Test 9: Invalid size format
        print("Test 9: Invalid size format (abc)")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        client_max_body_size abc;
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        # Invalid size format should be rejected
        assert not result["success"], (
            "Config with invalid size format should be rejected"
        )
        print("✓ Invalid size format rejected\n")

        # Test 10: Unknown directive
        print("Test 10: Unknown directive")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        unknown_directive value;
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with unknown directive should be rejected"
        print("✓ Unknown directive rejected\n")

        # Test 11: Directive in wrong context (listen in location)
        print("Test 11: Directive in wrong context (listen in location)")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        
        location / {{
            listen 9090;
        }}
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with wrong context should be rejected"
        print("✓ Wrong context rejected\n")

        # Test 12: Missing required argument
        print("Test 12: Missing required argument")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        client_max_body_size;
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with missing argument should be rejected"
        print("✓ Missing argument rejected\n")

        # Test 13: Empty configuration file
        print("Test 13: Empty configuration file")
        bad_conf = ""
        result = self.test_config(bad_conf, webserv_binary)
        # Empty configs should be rejected (no valid configuration)
        assert not result["success"], "Empty config should be rejected"
        print("✓ Empty config rejected\n")

        # Test 14: No server block
        print("Test 14: No server block in http")
        bad_conf = """http {
    worker_connections 1024;
}"""
        result = self.test_config(bad_conf, webserv_binary)
        # HTTP block without server should be rejected
        assert not result["success"], "Config without server block should be rejected"
        print("✓ No server block rejected\n")

        # Test 15: Duplicate conflicting directives
        print("Test 15: Multiple conflicting values")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        client_max_body_size 1m;
        client_max_body_size 10m;
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        # Duplicate directives may override (nginx-like) - accept either behavior
        print(
            f"  Duplicate directive: {'accepted (override)' if result['success'] else 'rejected'}\n"
        )

        # Test 16: Invalid characters in directive name
        print("Test 16: Invalid characters in directive name")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        client@max!body#size 1m;
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with invalid chars in directive should be rejected"
        )
        print("✓ Invalid directive characters rejected\n")

        # Test 17: Invalid nesting (server in location)
        print("Test 17: Invalid nesting (server in location)")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        server_name localhost;
        root {self.www_dir};
        
        location / {{
            server {{
                listen 9090;
            }}
        }}
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with invalid nesting should be rejected"
        print("✓ Invalid nesting rejected\n")

        # Test 18: Multiple http blocks (might be valid or not)
        print("Test 18: Multiple http blocks")
        bad_conf = f"""http {{
    server {{
        listen {self.get_test_port()};
        root {self.www_dir};
    }}
}}
http {{
    server {{
        listen 9090;
        root {self.www_dir};
    }}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        # Multiple http blocks should be rejected (nginx-like behavior)
        assert not result["success"], (
            "Config with multiple http blocks should be rejected"
        )
        print("✓ Multiple http blocks rejected\n")

        print("✓ All bad config tests completed!")

    def test_config(self, config_content, webserv_binary):
        """
        Test a configuration by trying to start the server with it.

        Returns:
            dict: {"success": bool, "stderr": str, "stdout": str}
        """
        # Create temporary config file
        with tempfile.NamedTemporaryFile(mode="w", suffix=".conf", delete=False) as f:
            f.write(config_content)
            temp_conf = f.name

        process = None
        try:
            # Try to start server with this config (will exit immediately or fail)
            # Use a very short timeout and expect it to fail for bad configs
            process = subprocess.Popen(
                [webserv_binary, temp_conf],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            # Wait for process to exit or timeout
            try:
                stdout, stderr = process.communicate(timeout=2)
                # If it returns 0, config was accepted
                # If it returns non-zero, config was rejected
                success = process.returncode == 0

                return {
                    "success": success,
                    "stdout": stdout,
                    "stderr": stderr,
                    "returncode": process.returncode,
                }
            except subprocess.TimeoutExpired:
                # If it times out, server is running = config accepted
                # Kill the server
                process.kill()
                try:
                    process.wait(timeout=1)
                except:
                    process.terminate()
                    try:
                        process.wait(timeout=1)
                    except:
                        pass
                # Give the OS time to release the port
                import time

                time.sleep(0.5)
                return {"success": True, "stdout": "", "stderr": "", "returncode": 0}

        except Exception as e:
            if process:
                try:
                    process.kill()
                    process.wait(timeout=1)
                except:
                    pass
            return {"success": False, "stdout": "", "stderr": str(e), "returncode": -1}
        finally:
            # Clean up temp file
            try:
                os.unlink(temp_conf)
            except:
                pass

    def get_webserv_binary(self):
        """Get path to webserv binary."""
        # Try common locations
        possible_paths = [
            os.path.join(self._workspace_root, "webserv"),
            os.path.join(self._workspace_root, "build/webserv"),
            os.path.join(self._workspace_root, "bin/webserv"),
            "./webserv",
        ]

        for path in possible_paths:
            if os.path.isfile(path) and os.access(path, os.X_OK):
                return path

        # Default to "webserv" and hope it's in PATH
        return "webserv"
