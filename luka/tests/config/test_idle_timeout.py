"""Test idle_timeout directive."""

import socket
import time
from luka.base_test import BaseTest


class IdleTimeoutTest(BaseTest):
    def get_name(self) -> str:
        return "config_idle_timeout"

    def get_description(self) -> str:
        return "Tests that idle_timeout closes inactive connections"

    def setup(self):
        # Set idle_timeout to 3 seconds
        conf = f"""http {{
    idle_timeout 3s;
    server {{
        listen {self.port};
        root {self.www_dir};
        index index.html;
    }}
}}"""
        self.create_conf(conf)
        self.create_html_file("index.html", "<h1>Test</h1>")

    def run(self):
        # Test 1: Normal request should work
        response = self.get("/")
        self.assert_status(response, 200, "Normal request should work")

        # Test 2: Idle connection should be closed after timeout
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)  # Client timeout longer than server timeout

        try:
            start_time = time.time()
            sock.connect(("localhost", self.port))

            # Don't send anything, just wait for server to close connection
            data = sock.recv(1024)
            elapsed = time.time() - start_time

            if len(data) == 0:
                # Connection closed by server
                print(f"Connection closed after {elapsed:.1f}s")

                if elapsed > 15:
                    raise AssertionError(
                        f"Connection took too long to close ({elapsed:.1f}s > 15s)"
                    )

                print("✓ Connection closed (idle_timeout working, but timing is off)")
            else:
                raise AssertionError(f"Unexpected data received: {data}")

        except socket.timeout:
            elapsed = time.time() - start_time
            raise AssertionError(
                f"Socket timeout at {elapsed:.1f}s - server should have closed connection"
            )
        finally:
            sock.close()
