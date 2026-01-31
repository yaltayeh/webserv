"""Test request with malformed headers (no colon separator)."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class InvalidHeaderFormatTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_invalid_header_format"

    def get_description(self) -> str:
        return "Tests request with malformed headers (no colon separator)."

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

        # Invalid header format (no colon)
        response = self.send_raw_request("GET / HTTP/1.1\r\nHost localhost\r\n\r\n")
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line or "200" in status_line, (
            "Invalid header format should return 400 or be ignored"
        )
