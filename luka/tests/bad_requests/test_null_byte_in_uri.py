"""Test request with NULL byte in URI."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class NullByteInUriTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_null_byte_in_uri"

    def get_description(self) -> str:
        return "Tests request with NULL byte in URI."

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

        # NULL byte in URI
        response = self.send_raw_request(
            b"GET /test\x00.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
        )
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line or "404" in status_line, (
            f"NULL byte in URI should return 400/404, got {status_line}"
        )
