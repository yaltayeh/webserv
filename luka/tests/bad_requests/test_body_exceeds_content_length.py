"""Test POST with body exceeding declared Content-Length."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class BodyExceedsContentLengthTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_body_exceeds_content_length"

    def get_description(self) -> str:
        return "Tests POST with body exceeding declared Content-Length."

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

        # POST with body exceeding Content-Length
        request = (
            "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nHelloWorld"
        )
        response = self.send_raw_request(request)
        assert b"HTTP/" in response, "Should handle Content-Length correctly"
