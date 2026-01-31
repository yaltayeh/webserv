"""Test minimal HTTP/1.0 request with no headers."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class MinimalHttpRequestTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_minimal_http_request"

    def get_description(self) -> str:
        return "Tests minimal HTTP/1.0 request with no headers (should work)."

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

        # Test minimal HTTP/1.0 request (no headers required)
        response = self.send_raw_request("GET / HTTP/1.0\r\n\r\n")
        assert b"HTTP/" in response, "Minimal HTTP/1.0 request should work"
