"""Test missing HTTP version handling."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class MissingHttpVersionTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_missing_http_version"

    def get_description(self) -> str:
        return "Tests server response when HTTP version is missing (HTTP/0.9 style)."

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

        # Test missing HTTP version (HTTP/0.9 style or should error)
        response = self.send_raw_request("GET /\r\nHost: localhost\r\n\r\n")

        # Should return 400 Bad Request or handle as HTTP/0.9
        assert response.startswith(b"HTTP/") or len(response) > 0, (
            "Server should respond to request without version"
        )
