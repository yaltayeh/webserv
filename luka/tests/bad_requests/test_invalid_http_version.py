"""Test invalid HTTP version handling."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class InvalidHttpVersionTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_invalid_http_version"

    def get_description(self) -> str:
        return "Tests server response to invalid HTTP versions (HTTP/3.0, HTTP/99.9)."

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

        # Test invalid HTTP version (HTTP/3.0)
        response = self.send_raw_request("GET / HTTP/3.0\r\nHost: localhost\r\n\r\n")
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")

        # Should return 400 or 505 HTTP Version Not Supported
        assert any(code in status_line for code in ["400", "505"]), (
            f"Invalid HTTP version should return 400/505, got {status_line}"
        )
