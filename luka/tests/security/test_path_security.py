"""
Path security test.
Tests that the server properly handles path traversal attempts and other security issues (HTTP/1.0).
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class PathSecurityTest(BaseTest):
    def get_name(self) -> str:
        return "path_security"

    def get_description(self) -> str:
        return "Tests path traversal protection and path security (HTTP/1.0)."

    def setup(self):
        # Create directory structure
        self.create_html_file(
            "index.html", HTMLBuilder.index_page("Path Security Test")
        )
        self.create_file("public.txt", "Public content")

        # Create nested directories
        self.create_directory("public")
        self.create_file("data.txt", "Public data", subdir="public")

        self.create_directory("public/nested")
        self.create_file("nested.txt", "Nested content", subdir="public/nested")

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
        # Test 1: Normal access to files
        response = self.get("/public.txt")
        self.assert_status(response, 200, "Normal file access should work")

        # Test 2: Normal access to nested file
        response_nested = self.get("/public/nested/nested.txt")
        self.assert_status(
            response_nested, 200, "Normal nested file access should work"
        )

        # Test 3: Path traversal attempt - ../
        response_traverse1 = self.get("/../../../etc/passwd")
        # Should return 403 Forbidden or 404 Not Found (NOT 200)
        assert response_traverse1.status_code in [403, 404], (
            f"Path traversal should be blocked, got {response_traverse1.status_code}"
        )

        # Test 4: Path traversal from nested location
        response_traverse2 = self.get("/public/../../etc/passwd")
        assert response_traverse2.status_code in [403, 404], (
            f"Nested path traversal should be blocked, got {response_traverse2.status_code}"
        )

        # Test 5: Multiple ../ attempts
        response_traverse3 = self.get("/../../../../../../../../etc/passwd")
        assert response_traverse3.status_code in [403, 404], (
            f"Multiple path traversal should be blocked, got {response_traverse3.status_code}"
        )

        # Test 6: Encoded path traversal %2e%2e%2f (../)
        response_encoded = self.get("/%2e%2e/%2e%2e/%2e%2e/etc/passwd")
        assert response_encoded.status_code in [403, 404], (
            f"Encoded path traversal should be blocked, got {response_encoded.status_code}"
        )

        # Test 7: Double slashes // in path
        response_double_slash = self.get("//public//data.txt")
        # Should either normalize and return 200, or return 404
        assert response_double_slash.status_code in [200, 404], (
            f"Double slash should be handled properly, got {response_double_slash.status_code}"
        )

        # Test 8: Trailing slashes
        response_trailing = self.get("/public/")
        # Should return directory listing or index, or 404/403
        assert response_trailing.status_code in [200, 403, 404], (
            f"Trailing slash should be handled, got {response_trailing.status_code}"
        )

        # Test 9: Dot segments in path
        response_dot = self.get("/public/./data.txt")
        # Should normalize and return the file or 404
        assert response_dot.status_code in [200, 404], (
            f"Dot segment should be normalized, got {response_dot.status_code}"
        )

        # Test 10: Ensure legitimate nested access still works
        response_legit = self.get("/public/data.txt")
        self.assert_status(
            response_legit, 200, "Legitimate nested access should still work"
        )
        self.assert_contains(response_legit.text, "Public data")
