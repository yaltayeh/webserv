"""
Base class for configuration validation tests.
Provides utilities for testing webserv configuration files.
"""

from luka.base_test import BaseTest
import subprocess
import os
import tempfile
import random


class ConfigTestBase(BaseTest):
    """Base class for config tests with helper methods."""

    def setup(self):
        """Setup method for config tests."""
        # Use a base port for config tests, incrementing for each test
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

    def test_config(self, config_content, webserv_binary):
        """
        Test a configuration by trying to start the server with it.

        Returns:
                dict: {"success": bool, "stderr": str, "stdout": str, "returncode": int}
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
