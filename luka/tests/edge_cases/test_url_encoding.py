"""
URL encoding test.
Tests that the server handles URL-encoded characters correctly (HTTP/1.0).
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class UrlEncodingTest(BaseTest):
    def get_name(self) -> str:
        return "url_encoding"

    def get_description(self) -> str:
        return (
            "Tests URL encoding/decoding for special characters and spaces (HTTP/1.0)."
        )

    def setup(self):
        # Create files with special characters in names (that can be URL encoded)
        self.create_html_file("index.html", HTMLBuilder.index_page("URL Encoding Test"))
        self.create_html_file(
            "test file.html",
            HTMLBuilder.simple_page("Spaces", "<h1>File with spaces</h1>"),
        )
        self.create_file("file with spaces.txt", "Content with spaces in filename")

        # Create directory and files with special names
        self.create_directory("test dir")
        self.create_html_file(
            "nested.html",
            HTMLBuilder.simple_page("Nested", "<h1>Nested in spaced dir</h1>"),
            subdir="test dir",
        )

        # Create files with various characters
        self.create_file("data.txt", "Simple data file")
        self.create_file("file-with-dashes.txt", "File with dashes")
        self.create_file("file_with_underscores.txt", "File with underscores")

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
        # Test 1: Normal file without special characters
        response = self.get("/data.txt")
        self.assert_status(response, 200, "Normal filename should work")
        self.assert_contains(response.text, "Simple data")

        # Test 2: File with spaces - URL encoded with %20
        response_space = self.get("/test%20file.html")
        self.assert_status(response_space, 200, "URL-encoded space (%20) should work")
        self.assert_contains(response_space.text, "File with spaces")

        # Test 3: File with spaces in txt format
        response_txt_space = self.get("/file%20with%20spaces.txt")
        self.assert_status(
            response_txt_space, 200, "Text file with URL-encoded spaces should work"
        )
        self.assert_contains(response_txt_space.text, "Content with spaces")

        # Test 4: Nested file in directory with spaces
        response_nested = self.get("/test%20dir/nested.html")
        self.assert_status(
            response_nested, 200, "Nested file in spaced directory should work"
        )
        self.assert_contains(response_nested.text, "Nested in spaced dir")

        # Test 5: File with dashes (no encoding needed)
        response_dash = self.get("/file-with-dashes.txt")
        self.assert_status(response_dash, 200, "File with dashes should work")

        # Test 6: File with underscores (no encoding needed)
        response_underscore = self.get("/file_with_underscores.txt")
        self.assert_status(
            response_underscore, 200, "File with underscores should work"
        )

        # Test 7: Query string with encoded parameters
        response_query = self.get("/index.html?name=test%20value&id=123")
        # Should still return the page (query string might be ignored for static files)
        self.assert_status(
            response_query, 200, "URL with encoded query parameters should work"
        )

        # Test 8: Multiple encoded characters
        # Note: Server should decode %20 to space
        response_multi = self.get(
            "/test%20file.html?param1=value%20one&param2=value%20two"
        )
        self.assert_status(
            response_multi, 200, "Multiple encoded characters should work"
        )
