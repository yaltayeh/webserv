"""Test empty request (only CRLF)."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class EmptyRequestTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_empty_request"

    def get_description(self) -> str:
        return "Tests empty request (only CRLF)."

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

        # Empty request
        response = self.send_raw_request("\r\n", wait_time=1)
        assert len(response) == 0 or b"400" in response, (
            "Empty request should close connection or return 400"
        )
