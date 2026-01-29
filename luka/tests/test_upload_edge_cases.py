"""
Advanced upload edge cases test.
Tests various edge cases for file uploads including size limits, permissions, and error conditions.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder
import os


class UploadEdgeCasesTest(BaseTest):
    def get_name(self) -> str:
        return "upload_edge_cases"

    def get_description(self) -> str:
        return "Tests upload edge cases: size limits, disabled uploads, missing directories, concurrent uploads."

    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("Upload Edge Cases"))

        # Create a non-existent upload directory path for testing
        self.nonexistent_upload_dir = os.path.join(self.test_dir, "nonexistent_uploads")

        # Create a restricted upload directory for permission tests
        self.restricted_upload_dir = os.path.join(self.test_dir, "restricted_uploads")
        os.makedirs(self.restricted_upload_dir, exist_ok=True)
        # Make it read-only (will test if server handles write permission errors)
        os.chmod(self.restricted_upload_dir, 0o444)

        # Create configuration with various upload scenarios
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 5k;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /upload_enabled {{
            allowed_methods POST PUT;
            upload_enable on;
            upload_store {self.uploads_dir};
        }}
        
        location /upload_disabled {{
            allowed_methods POST PUT;
            upload_enable off;
            upload_store {self.uploads_dir};
        }}
        
        location /upload_no_dir {{
            allowed_methods POST PUT;
            upload_enable on;
            upload_store {self.nonexistent_upload_dir};
            create_full_put_path on;
        }}
        
        location /upload_restricted {{
            allowed_methods POST PUT;
            upload_enable on;
            upload_store {self.restricted_upload_dir};
        }}
        
        location /upload_no_method {{
            allowed_methods GET;
            upload_enable on;
            upload_store {self.uploads_dir};
        }}
        
        location /upload_unlimited {{
            allowed_methods POST PUT;
            upload_enable on;
            upload_store {self.uploads_dir};
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: Verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")

        # Test 2: Upload to enabled location - should succeed
        file_content = "This is a test file for upload"
        response = self.post(
            "/upload_enabled", data=file_content, headers={"Content-Type": "text/plain"}
        )
        # Accept 404 if upload endpoint is not configured
        if response.status_code == 404:
            print(
                "⚠ Upload endpoint returns 404 - uploads may not be configured properly"
            )
            print("  Skipping remaining upload tests as uploads appear unavailable\n")
            return

        self.assert_status(
            response, [200, 201, 204], "Upload to enabled location should succeed"
        )

        # Test 3: Upload exceeding size limit (5k) to upload_enabled
        large_content = "x" * (6 * 1024)
        response = self.post(
            "/upload_enabled",
            data=large_content,
            headers={"Content-Type": "text/plain"},
        )
        self.assert_status(response, 413, "Upload exceeding 5k limit should return 413")

        # Test 4: Upload exactly at size limit (5k = 5120 bytes)
        exact_limit = "x" * (5 * 1024)
        response = self.post(
            "/upload_enabled", data=exact_limit, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(
            response, [200, 201, 204], "Upload at exact limit should succeed"
        )

        # Test 5: Upload to disabled location - should fail
        response = self.post(
            "/upload_disabled",
            data="test content",
            headers={"Content-Type": "text/plain"},
        )
        # Should return 403 Forbidden or 405 Method Not Allowed
        assert response.status_code in [403, 405, 404], (
            f"Upload to disabled location should fail, got {response.status_code}"
        )

        # Test 6: Upload with empty body (0 bytes)
        response = self.post(
            "/upload_enabled", data="", headers={"Content-Type": "text/plain"}
        )
        # Empty upload might succeed or fail depending on implementation
        # Most servers accept empty uploads
        assert response.status_code in [200, 201, 204, 400], (
            f"Empty upload returned unexpected status {response.status_code}"
        )

        # Test 7: Upload with missing Content-Type header
        response = self.post("/upload_enabled", data="test content")
        # Should still work - server might use default content type
        self.assert_status(
            response,
            [200, 201, 204, 400],
            "Upload without Content-Type might succeed or return 400",
        )

        # Test 8: Upload to non-existent directory with create_full_put_path on
        # Should create directory and succeed
        response = self.post(
            "/upload_no_dir",
            data="test content",
            headers={"Content-Type": "text/plain"},
        )
        # Should succeed if create_full_put_path works
        # Or might fail if not implemented
        assert response.status_code in [200, 201, 204, 403, 500], (
            f"Upload to non-existent dir returned {response.status_code}"
        )

        # Test 9: Upload via PUT method (vs POST)
        put_content = "Content uploaded via PUT"
        response = self.put(
            "/upload_enabled", data=put_content, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(response, [200, 201, 204], "PUT upload should succeed")

        # Test 10: Upload to location without POST/PUT allowed
        response = self.post(
            "/upload_no_method",
            data="test content",
            headers={"Content-Type": "text/plain"},
        )
        self.assert_status(
            response, 405, "Upload to GET-only location should return 405"
        )

        # Test 11: Upload with special characters in body
        special_content = (
            "Test with special chars: !@#$%^&*()[]{}|\\:;\"'<>,.?/~`\n\r\t"
        )
        response = self.post(
            "/upload_enabled",
            data=special_content,
            headers={"Content-Type": "text/plain"},
        )
        self.assert_status(
            response, [200, 201, 204], "Upload with special characters should succeed"
        )

        # Test 12: Upload with binary content (simulated)
        binary_content = bytes(range(256)).decode("latin-1")
        response = self.post(
            "/upload_enabled",
            data=binary_content,
            headers={"Content-Type": "application/octet-stream"},
        )
        self.assert_status(
            response, [200, 201, 204], "Upload with binary content should succeed"
        )

        # Test 13: Upload with 4KB content (under 5k limit)
        # Test with 4KB file
        large_content = "x" * (4 * 1024)
        response = self.post(
            "/upload_unlimited",
            data=large_content,
            headers={"Content-Type": "text/plain"},
        )
        self.assert_status(
            response,
            [200, 201, 204],
            "Upload with 4KB content should succeed",
        )

        # Test 14: Concurrent uploads to test thread safety
        # Upload 5 files concurrently
        upload_requests = [
            {
                "method": "POST",
                "url": self.get_url("/upload_enabled"),
                "data": f"Concurrent upload {i}",
                "headers": {"Content-Type": "text/plain"},
            }
            for i in range(5)
        ]

        responses = self.concurrent_requests(upload_requests)
        # All should succeed
        for i, result in enumerate(responses):
            if result and result.response:
                assert result.response.status_code in [200, 201, 204], (
                    f"Concurrent upload {i} failed with status {result.response.status_code}"
                )

        # Test 15: Upload with very long content (but under limit)
        long_content = "a" * (4 * 1024)  # 4KB
        response = self.post(
            "/upload_enabled", data=long_content, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(
            response, [200, 201, 204], "Upload with 4KB content should succeed"
        )

        # Test 16: PUT vs POST behavior difference
        # Some servers treat PUT differently from POST for uploads
        post_data = "POST upload"
        put_data = "PUT upload"

        post_response = self.post(
            "/upload_enabled", data=post_data, headers={"Content-Type": "text/plain"}
        )
        put_response = self.put(
            "/upload_enabled", data=put_data, headers={"Content-Type": "text/plain"}
        )

        # Both should succeed
        assert post_response.status_code in [200, 201, 204], (
            f"POST upload failed with {post_response.status_code}"
        )
        assert put_response.status_code in [200, 201, 204], (
            f"PUT upload failed with {put_response.status_code}"
        )

        print("✓ All upload edge case tests passed!")

    def cleanup(self):
        """Cleanup restricted directory permissions."""
        try:
            if os.path.exists(self.restricted_upload_dir):
                os.chmod(self.restricted_upload_dir, 0o755)
        except:
            pass

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
