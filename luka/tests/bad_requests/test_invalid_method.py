"""Test invalid HTTP method handling."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class InvalidMethodTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_invalid_method"

    def get_description(self) -> str:
        return "Tests server response to invalid HTTP methods (e.g., GETT, HELLO)."

    def setup(self):
        self.create_html_file("index.html", HTMLBuilder.index_page("Test"))
        self.create_file("test.txt", "Test file content")

        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST PUT DELETE;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")

        # Test invalid HTTP method (should return 400 or 405 or 501)
        response = self.send_raw_request("GETT / HTTP/1.1\r\nHost: localhost\r\n\r\n")
        assert response.startswith(b"HTTP/1."), "Should receive HTTP response"

        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert any(code in status_line for code in ["400", "405", "501"]), (
            f"Invalid method should return 400/405/501, got {status_line}"
        )
