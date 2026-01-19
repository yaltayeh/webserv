"""
HTTP/1.0 protocol test.
Tests HTTP/1.0 protocol compliance and proper request handling.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class HttpVersionsTest(BaseTest):
    def get_name(self) -> str:
        return "http_versions"

    def get_description(self) -> str:
        return "Tests HTTP/1.0 protocol compliance (all requests use HTTP/1.0)."

    def setup(self):
        # Create test files
        self.create_html_file("index.html", HTMLBuilder.index_page("HTTP Version Test"))
        self.create_html_file(
            "test.html", HTMLBuilder.simple_page("Test", "<h1>Test Content</h1>")
        )
        self.create_file("data.txt", "Sample text data for testing.")

        # Create configuration
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
        # All tests use HTTP/1.0 as required

        # Test basic GET request
        response = self.get("/")
        self.assert_status(response, 200, "HTTP/1.0 GET should succeed")
        self.assert_contains(response.text, "HTTP Version Test")

        # Test file request
        response_file = self.get("/test.html")
        self.assert_status(response_file, 200, "HTTP/1.0 file request should succeed")
        self.assert_contains(response_file.text, "Test Content")

        # Test text file
        response_txt = self.get("/data.txt")
        self.assert_status(response_txt, 200, "HTTP/1.0 text file should succeed")
        self.assert_contains(response_txt.text, "Sample text data")

        # Test POST request
        response_post = self.post("/test.html", data="key=value")
        if response_post:
            assert response_post.status_code in [200, 201, 204, 405], (
                f"HTTP/1.0 POST should be handled, got {response_post.status_code}"
            )

        # Verify Content-Length header is present and accurate
        if hasattr(response, "headers") and "Content-Length" in response.headers:
            content_length = int(response.headers["Content-Length"])
            actual_length = len(response.content)
            self.assert_equal(
                actual_length,
                content_length,
                "Content-Length should match actual content length",
            )

        # Test multiple sequential requests work correctly
        response2 = self.get("/")
        self.assert_status(response2, 200, "Second HTTP/1.0 request should succeed")

        response3 = self.get("/test.html")
        self.assert_status(response3, 200, "Third HTTP/1.0 request should succeed")
