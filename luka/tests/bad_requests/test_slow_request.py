"""Test partial request sent slowly (slowloris-style)."""

import socket
import time
from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class SlowRequestTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_slow_request"

    def get_description(self) -> str:
        return "Tests partial request sent slowly (slowloris-style)."

    def setup(self):
        self.create_html_file("index.html", HTMLBuilder.index_page("Test"))

        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")

        # Slow request (partial headers)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        try:
            sock.connect(("localhost", self.port))
            sock.send(b"GET / HTTP/1.1\r\n")
            time.sleep(0.5)
            sock.send(b"Host: localhost\r\n")
            time.sleep(0.5)
            sock.send(b"\r\n")
            response = sock.recv(4096)
            assert b"HTTP/" in response, "Slow request should eventually complete"
        finally:
            sock.close()
