"""Test POST with Content-Length but no body."""

import socket
from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class ContentLengthMismatchTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_content_length_mismatch"

    def get_description(self) -> str:
        return "Tests POST with Content-Length but no body."

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

        # POST with Content-Length but no body
        try:
            response = self.send_raw_request(
                "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\n",
                wait_time=2,
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            assert "HTTP/" in status_line, "Should receive HTTP response"
        except socket.timeout:
            pass  # Timeout is expected
