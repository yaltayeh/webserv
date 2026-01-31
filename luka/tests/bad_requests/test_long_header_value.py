"""Test request with extremely long header value (16KB)."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class LongHeaderValueTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_long_header_value"

    def get_description(self) -> str:
        return "Tests request with extremely long header value (16KB)."

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

        # Extremely long header value
        long_value = "x" * 16384
        response = self.send_raw_request(
            f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Long-Header: {long_value}\r\n\r\n"
        )
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, "Should receive HTTP response"
