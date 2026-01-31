"""
DELETE method test.
Tests if the server handles DELETE requests correctly according to HTTP spec.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class DeleteMethodTest(BaseTest):
    def get_name(self) -> str:
        return "delete_method"

    def get_description(self) -> str:
        return "Tests DELETE method functionality - delete files, handle errors, and method restrictions."

    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("Delete Test"))

        # Create files that can be deleted
        self.create_html_file(
            "deletable.html",
            HTMLBuilder.simple_page("Deletable", "<h1>This file can be deleted</h1>"),
        )
        self.create_file("test.txt", "This is a test file for deletion.")

        # Create a subdirectory with a file
        self.create_directory("deletable_dir")
        self.create_html_file(
            "file_in_dir.html",
            HTMLBuilder.simple_page("In Dir", "<h1>File in directory</h1>"),
            subdir="deletable_dir",
        )

        # Create configuration with DELETE support
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST DELETE;
        }}
        
        location /protected/ {{
            allowed_methods GET POST;
        }}
        
        location /readonly/ {{
            allowed_methods GET;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: Verify files exist before deletion
        response = self.get("/deletable.html")
        self.assert_status(response, 200, "File should exist before deletion")

        # Test 2: DELETE existing file - should succeed (200, 202, or 204)
        response_delete = self.delete("/deletable.html")
        if response_delete:
            assert response_delete.status_code in [200, 202, 204], (
                f"DELETE should return 200/202/204, got {response_delete.status_code}"
            )

        # Test 3: Verify file is deleted - GET should return 404
        response_after = self.get("/deletable.html")
        self.assert_status(response_after, 404, "File should not exist after deletion")

        # Test 4: DELETE non-existent file - should return 404
        response_not_found = self.delete("/nonexistent.html")
        self.assert_status(
            response_not_found, 404, "Deleting non-existent file should return 404"
        )

        # Test 5: DELETE in location without DELETE allowed - should return 405
        response_not_allowed = self.delete("/readonly/test.html")
        self.assert_status(
            response_not_allowed, 405, "DELETE on GET-only location should return 405"
        )

        # Test 6: DELETE another file to verify consistency
        response_txt = self.get("/test.txt")
        self.assert_status(response_txt, 200, "Text file should exist")

        response_delete_txt = self.delete("/test.txt")
        if response_delete_txt:
            assert response_delete_txt.status_code in [200, 202, 204], (
                f"DELETE text file should succeed, got {response_delete_txt.status_code}"
            )

    def delete(self, path: str, headers=None):
        """Make a DELETE request - will be injected by runner."""
        raise NotImplementedError("DELETE method will be injected by runner")
