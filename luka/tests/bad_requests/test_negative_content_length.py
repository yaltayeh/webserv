"""Test POST with negative Content-Length value."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class NegativeContentLengthTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_negative_content_length"

    def get_description(self) -> str:
        return "Tests POST with negative Content-Length value."

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

        # Negative Content-Length
        response = self.send_raw_request(
            "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\n"
        )
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line, (
            f"Negative Content-Length should return 400, got {status_line}"
        )
