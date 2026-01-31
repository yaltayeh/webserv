"""Test request with extremely long URI (16KB)."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class LongUriTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_long_uri"

    def get_description(self) -> str:
        return "Tests request with extremely long URI (16KB)."

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

        # Extremely long URI
        long_uri = "/" + "x" * 16384
        response = self.send_raw_request(
            f"GET {long_uri} HTTP/1.1\r\nHost: localhost\r\n\r\n"
        )
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert any(code in status_line for code in ["414", "400"]), (
            f"Long URI should return 414/400, got {status_line}"
        )
