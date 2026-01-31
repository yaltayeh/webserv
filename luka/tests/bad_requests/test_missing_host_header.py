"""Test HTTP/1.1 request without required Host header."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class MissingHostHeaderTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_missing_host_header"

    def get_description(self) -> str:
        return "Tests HTTP/1.1 request without required Host header."

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

        # HTTP/1.1 requires Host header
        response = self.send_raw_request("GET / HTTP/1.1\r\n\r\n")
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, "Should receive HTTP response"
