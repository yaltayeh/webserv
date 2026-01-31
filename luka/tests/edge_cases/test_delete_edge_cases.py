"""
Advanced DELETE edge cases test.
Tests various edge cases for DELETE method including security, race conditions, and special paths.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder
import os


class DeleteEdgeCasesTest(BaseTest):
    def get_name(self) -> str:
        return "delete_edge_cases"

    def get_description(self) -> str:
        return "Tests DELETE edge cases: directories, path traversal, concurrent deletes, special paths."

    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("DELETE Edge Cases"))

        # Create various files for deletion testing
        self.create_file("deletable1.txt", "File 1 content")
        self.create_file("deletable2.txt", "File 2 content")
        self.create_file("deletable3.html", "<h1>File 3</h1>")

        # Create file with special characters in name
        self.create_file("file with spaces.txt", "Spaces in filename")
        self.create_file("file-with-dashes.txt", "Dashes in filename")
        self.create_file("file_with_underscores.txt", "Underscores in filename")

        # Create nested directory structure
        self.create_directory("subdir1")
        self.create_file("file1.txt", "File in subdir1", subdir="subdir1")
        self.create_file("file2.txt", "File 2 in subdir1", subdir="subdir1")

        self.create_directory("subdir2")
        self.create_file("nested.txt", "Nested file", subdir="subdir2")

        # Create deep nesting
        self.create_directory("deep/nested/structure")
        self.create_file(
            "deep_file.txt", "Deep nested file", subdir="deep/nested/structure"
        )

        # Create directory with multiple files for directory deletion test
        self.create_directory("dir_to_delete")
        self.create_file("file_in_dir.txt", "File in directory", subdir="dir_to_delete")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST DELETE;
        }}
        
        location /protected {{
            allowed_methods GET POST;
        }}
        
        location /readonly {{
            allowed_methods GET;
        }}
        
        location /subdir1 {{
            allowed_methods GET DELETE;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: Verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")

        # Test 2: DELETE regular file
        response = self.get("/deletable1.txt")
        self.assert_status(response, 200, "File should exist before delete")

        response = self.delete("/deletable1.txt")
        self.assert_status(response, [200, 202, 204], "DELETE should succeed")

        response = self.get("/deletable1.txt")
        self.assert_status(response, 404, "File should not exist after delete")

        # Test 3: Double DELETE - delete same file twice
        response = self.get("/deletable2.txt")
        self.assert_status(response, 200, "File should exist")

        # First delete
        response = self.delete("/deletable2.txt")
        self.assert_status(response, [200, 202, 204], "First DELETE should succeed")

        # Second delete on already deleted file
        response = self.delete("/deletable2.txt")
        self.assert_status(response, 404, "Second DELETE should return 404")

        # Test 4: DELETE non-existent file
        response = self.delete("/never_existed.txt")
        self.assert_status(response, 404, "DELETE non-existent file should return 404")

        # Test 5: DELETE with trailing slash
        response = self.delete("/deletable3.html/")
        # Might return 404 or handle gracefully depending on implementation
        assert response.status_code in [200, 202, 204, 404, 400], (
            f"DELETE with trailing slash returned {response.status_code}"
        )

        # Test 6: DELETE directory (not a file)
        response = self.delete("/dir_to_delete")
        # Server might reject directory deletion or allow it
        # HTTP spec doesn't clearly define DELETE on directories
        assert response.status_code in [200, 202, 204, 403, 404, 405, 409], (
            f"DELETE directory returned {response.status_code}"
        )

        # Test 7: DELETE with path traversal attempt - ../ (security test)
        response = self.delete("/../etc/passwd")
        # Should be rejected - either 403 or 404
        assert response.status_code in [400, 403, 404], (
            f"Path traversal should be blocked, got {response.status_code}"
        )

        # Test 8: Another path traversal attempt
        response = self.delete("/subdir1/../../sensitive.txt")
        assert response.status_code in [400, 403, 404], (
            f"Path traversal should be blocked, got {response.status_code}"
        )

        # Test 9: DELETE root path / (should be forbidden)
        response = self.delete("/")
        assert response.status_code in [403, 404, 405], (
            f"DELETE root should be forbidden, got {response.status_code}"
        )

        # Test 10: DELETE file in subdirectory
        response = self.get("/subdir1/file1.txt")
        self.assert_status(response, 200, "Subdirectory file should exist")

        response = self.delete("/subdir1/file1.txt")
        self.assert_status(
            response, [200, 202, 204], "DELETE file in subdirectory should succeed"
        )

        response = self.get("/subdir1/file1.txt")
        self.assert_status(response, 404, "File should be deleted")

        # Test 11: DELETE with URL encoding
        # Try to delete "file with spaces.txt" using URL encoding
        response = self.delete("/file%20with%20spaces.txt")
        # Should work if server handles URL decoding
        assert response.status_code in [200, 202, 204, 404], (
            f"DELETE with URL encoding returned {response.status_code}"
        )

        # Test 12: DELETE with query string (should ignore query string)
        response = self.delete("/subdir1/file2.txt?key=value&foo=bar")
        # Should delete the file, ignoring query string
        assert response.status_code in [200, 202, 204], (
            f"DELETE with query string returned {response.status_code}"
        )

        # Test 13: DELETE in protected location (DELETE not allowed)
        self.create_file("protected_file.txt", "Protected content", subdir="protected")
        response = self.delete("/protected/protected_file.txt")
        self.assert_status(
            response, 405, "DELETE in protected location should return 405"
        )

        # Test 14: DELETE in readonly location
        self.create_file("readonly_file.txt", "Readonly content", subdir="readonly")
        response = self.delete("/readonly/readonly_file.txt")
        self.assert_status(
            response, 405, "DELETE in readonly location should return 405"
        )

        # Test 15: Concurrent DELETE requests on different files
        # Create multiple files for concurrent deletion
        for i in range(5):
            self.create_file(f"concurrent_{i}.txt", f"Concurrent file {i}")

        delete_requests = [
            {"method": "DELETE", "url": self.get_url(f"/concurrent_{i}.txt")}
            for i in range(5)
        ]

        responses = self.concurrent_requests(delete_requests)
        # All should succeed
        for i, result in enumerate(responses):
            if result and result.response:
                assert result.response.status_code in [200, 202, 204], (
                    f"Concurrent DELETE {i} failed with {result.response.status_code}"
                )

        # Verify all files are deleted
        for i in range(5):
            response = self.get(f"/concurrent_{i}.txt")
            self.assert_status(response, 404, f"Concurrent file {i} should be deleted")

        # Test 16: Concurrent DELETE of same file (race condition)
        self.create_file("race_condition.txt", "Race condition test")

        # Try to delete the same file 3 times concurrently
        race_requests = [
            {"method": "DELETE", "url": self.get_url("/race_condition.txt")}
            for _ in range(3)
        ]

        responses = self.concurrent_requests(race_requests)
        # One should succeed (200/202/204), others should return 404
        success_count = sum(
            1
            for r in responses
            if r and r.response and r.response.status_code in [200, 202, 204]
        )
        fail_count = sum(
            1 for r in responses if r and r.response and r.response.status_code == 404
        )

        # At least one should succeed, others should fail with 404
        assert success_count >= 1, "At least one concurrent DELETE should succeed"
        # The file should be gone
        response = self.get("/race_condition.txt")
        self.assert_status(response, 404, "File should be deleted after race condition")

        # Test 17: DELETE deeply nested file
        response = self.delete("/deep/nested/structure/deep_file.txt")
        assert response.status_code in [200, 202, 204], (
            f"DELETE deeply nested file returned {response.status_code}"
        )

        # Test 18: DELETE with special characters in filename
        response = self.delete("/file-with-dashes.txt")
        assert response.status_code in [200, 202, 204], (
            "DELETE file with dashes should succeed"
        )

        response = self.delete("/file_with_underscores.txt")
        assert response.status_code in [200, 202, 204], (
            "DELETE file with underscores should succeed"
        )

        # Test 19: DELETE with double slashes in path
        self.create_file("double_slash.txt", "Double slash test")
        response = self.delete("//double_slash.txt")
        # Should normalize path and work
        assert response.status_code in [200, 202, 204, 404], (
            f"DELETE with double slash returned {response.status_code}"
        )

        # Test 20: DELETE with fragment identifier (should be ignored by server)
        self.create_file("fragment_test.txt", "Fragment test")
        response = self.delete("/fragment_test.txt#section")
        # Server should ignore fragment and delete the file
        assert response.status_code in [200, 202, 204], (
            f"DELETE with fragment returned {response.status_code}"
        )

        print("✓ All DELETE edge case tests passed!")

    def assert_status(self, response, expected_status, message: str = ""):
        """Custom assert_status that accepts list of valid status codes."""
        if response is None:
            raise AssertionError("HTTP request failed - response is None")

        if isinstance(expected_status, list):
            if response.status_code not in expected_status:
                raise AssertionError(
                    f"{message} - Expected one of {expected_status}, got {response.status_code}"
                )
        else:
            if response.status_code != expected_status:
                raise AssertionError(
                    f"{message} - Expected {expected_status}, got {response.status_code}"
                )
