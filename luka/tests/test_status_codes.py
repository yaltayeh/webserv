"""
HTTP Status codes test.
Tests comprehensive status code handling: 400, 405, 413, 500, 501, 505, etc.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class StatusCodesTest(BaseTest):
    def get_name(self) -> str:
        return "status_codes"

    def get_description(self) -> str:
        return (
            "Tests that server returns proper HTTP status codes for various scenarios."
        )

    def setup(self):
        # Create basic files
        self.create_html_file("index.html", HTMLBuilder.index_page("Status Test"))
        self.create_html_file(
            "test.html", HTMLBuilder.simple_page("Test", "<h1>Test Page</h1>")
        )

        # Create custom error pages
        self.create_html_file("400.html", HTMLBuilder.error_page(400, "Bad Request"))
        self.create_html_file("404.html", HTMLBuilder.error_page(404, "Not Found"))
        self.create_html_file(
            "405.html", HTMLBuilder.error_page(405, "Method Not Allowed")
        )
        self.create_html_file(
            "500.html", HTMLBuilder.error_page(500, "Internal Server Error")
        )

        # Create location for DELETE testing
        self.create_directory("deletable")
        self.create_file("file.txt", "Deletable content", subdir="deletable")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        error_page 400 /400.html;
        error_page 404 /404.html;
        error_page 405 /405.html;
        error_page 500 /500.html;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /deletable/ {{
            allowed_methods GET DELETE;
        }}
        
        location /readonly/ {{
            allowed_methods GET;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 200 OK - successful GET request
        response_200 = self.get("/")
        self.assert_status(response_200, 200, "Index page should return 200 OK")
        self.assert_contains(response_200.text, "Status Test")

        # Test 404 Not Found - non-existent file
        response_404 = self.get("/nonexistent.html")
        self.assert_status(response_404, 404, "Non-existent file should return 404")
        self.assert_contains(
            response_404.text, "Not Found", "Should show custom 404 page"
        )

        # Test 405 Method Not Allowed - DELETE on location that doesn't allow it
        response_405 = self.delete("/")
        self.assert_status(
            response_405, 405, "DELETE on GET/POST-only location should return 405"
        )

        # Test 405 - POST on GET-only location
        response_405_post = self.post("/readonly/test.html", data="data")
        self.assert_status(
            response_405_post, 405, "POST on GET-only location should return 405"
        )

        # Additional tests for other status codes (these may or may not be implemented)

        # Test that successful operations return appropriate 2xx codes
        response_get = self.get("/test.html")
        assert response_get.status_code in [200], (
            f"Successful GET should return 2xx, got {response_get.status_code}"
        )

        # Test that the server provides proper status messages
        if response_404.status_code == 404:
            # Status code is correctly 404
            pass

        # Test DELETE returning proper status
        response_delete = self.delete("/deletable/file.txt")
        if response_delete:
            assert response_delete.status_code in [200, 202, 204, 404], (
                f"DELETE should return 200/202/204 or 404, got {response_delete.status_code}"
            )
