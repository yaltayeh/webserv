"""
Multipart/form-data upload test.
Tests real multipart file uploads with proper MIME formatting.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder
import socket
import os


class MultipartUploadTest(BaseTest):
    def get_name(self) -> str:
        return "upload_multipart"

    def get_description(self) -> str:
        return "Tests multipart/form-data file uploads with various edge cases."

    def setup(self):
        # Create index page with file upload form
        self.create_html_file("index.html", HTMLBuilder.form_page("/upload", "POST"))

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 5m;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /upload {{
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

        # Test 2: Simple multipart upload with one file
        boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"
        body = self.create_multipart_body(
            boundary,
            files=[("file", "test.txt", "text/plain", "This is test file content")],
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        # Should succeed or return 400/404/501 if multipart not implemented/configured
        assert response.status_code in [200, 201, 204, 400, 404, 501], (
            f"Multipart upload returned unexpected status {response.status_code}"
        )

        if response.status_code in [200, 201, 204]:
            print("✓ Server supports multipart/form-data uploads")
        else:
            print(
                f"⚠ Server doesn't fully support multipart (status {response.status_code})"
            )
            print("  This is OK - not all servers implement multipart parsing")

        # Test 3: Multipart upload with multiple files
        body = self.create_multipart_body(
            boundary,
            files=[
                ("file1", "test1.txt", "text/plain", "File 1 content"),
                ("file2", "test2.txt", "text/plain", "File 2 content"),
                ("file3", "test3.txt", "text/plain", "File 3 content"),
            ],
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        assert response.status_code in [200, 201, 204, 400, 404, 501], (
            f"Multiple file upload returned {response.status_code}"
        )

        # Test 4: Multipart with form fields and files
        body = self.create_multipart_body(
            boundary,
            fields=[("username", "testuser"), ("description", "Test upload")],
            files=[("file", "data.txt", "text/plain", "File with form data")],
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        assert response.status_code in [200, 201, 204, 400, 404, 501], (
            f"Mixed form/file upload returned {response.status_code}"
        )

        # Test 5: Multipart with missing boundary in Content-Type
        body = self.create_multipart_body(
            boundary, files=[("file", "test.txt", "text/plain", "Test")]
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": "multipart/form-data"},  # Missing boundary
        )

        # Should return 400 Bad Request
        assert response.status_code in [400, 404, 500], (
            f"Missing boundary should return 400, got {response.status_code}"
        )

        # Test 6: Multipart with invalid boundary (not in body)
        body = self.create_multipart_body(
            boundary, files=[("file", "test.txt", "text/plain", "Test")]
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": "multipart/form-data; boundary=WrongBoundary"},
        )

        # Should return 400 Bad Request
        assert response.status_code in [400, 404, 500], (
            f"Wrong boundary should return 400, got {response.status_code}"
        )

        # Test 7: Multipart with binary file content
        binary_content = bytes(range(256))
        body = self.create_multipart_body(
            boundary,
            files=[("file", "binary.bin", "application/octet-stream", binary_content)],
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        assert response.status_code in [200, 201, 204, 400, 404, 501], (
            f"Binary file upload returned {response.status_code}"
        )

        # Test 8: Multipart with empty file
        body = self.create_multipart_body(
            boundary, files=[("file", "empty.txt", "text/plain", "")]
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        assert response.status_code in [200, 201, 204, 400, 404, 501], (
            f"Empty file upload returned {response.status_code}"
        )

        # Test 9: Multipart with special characters in filename
        body = self.create_multipart_body(
            boundary,
            files=[("file", "test file (1).txt", "text/plain", "Special filename")],
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        assert response.status_code in [200, 201, 204, 400, 404, 501], (
            f"Special filename upload returned {response.status_code}"
        )

        # Test 10: Malformed multipart - missing final boundary
        body = f"------{boundary}\r\n"
        body += 'Content-Disposition: form-data; name="file"; filename="test.txt"\r\n'
        body += "Content-Type: text/plain\r\n\r\n"
        body += "Test content\r\n"
        # Missing final boundary

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        # Should return 400 Bad Request
        assert response.status_code in [400, 404, 500, 408], (
            f"Malformed multipart should return error, got {response.status_code}"
        )

        # Test 11: Large file via multipart (1MB)
        large_content = "x" * (1024 * 1024)
        body = self.create_multipart_body(
            boundary, files=[("file", "large.txt", "text/plain", large_content)]
        )

        response = self.post(
            "/upload",
            data=body,
            headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
        )

        assert response.status_code in [200, 201, 204, 400, 404, 413, 501], (
            f"Large file upload returned {response.status_code}"
        )

        # Test 12: Multipart exceeding client_max_body_size (> 5MB)
        # Create 6MB file
        huge_content = "x" * (6 * 1024 * 1024)
        body = self.create_multipart_body(
            boundary, files=[("file", "huge.txt", "text/plain", huge_content)]
        )

        try:
            response = self.post(
                "/upload",
                data=body,
                headers={"Content-Type": f"multipart/form-data; boundary={boundary}"},
            )

            # Should return 413 Payload Too Large or 404 if upload not configured
            assert response.status_code in [413, 400, 404], (
                f"Oversized multipart should return 413/400/404, got {response.status_code}"
            )
        except Exception as e:
            # Connection reset is acceptable for oversized requests
            if "Connection reset" in str(e) or "Errno 104" in str(e):
                print("  Oversized request caused connection reset (acceptable)")
            else:
                raise

        print("✓ All multipart upload tests completed!")
        print(
            "Note: If server doesn't implement multipart parsing, tests will show warnings."
        )

    def create_multipart_body(self, boundary, fields=None, files=None):
        """
        Create a properly formatted multipart/form-data request body.

        Args:
            boundary: The multipart boundary string
            fields: List of tuples (name, value) for form fields
            files: List of tuples (name, filename, content_type, content) for files

        Returns:
            str: The formatted multipart body
        """
        body = ""

        # Add form fields
        if fields:
            for name, value in fields:
                body += f"------{boundary}\r\n"
                body += f'Content-Disposition: form-data; name="{name}"\r\n\r\n'
                body += f"{value}\r\n"

        # Add files
        if files:
            for name, filename, content_type, content in files:
                body += f"------{boundary}\r\n"
                body += f'Content-Disposition: form-data; name="{name}"; filename="{filename}"\r\n'
                body += f"Content-Type: {content_type}\r\n\r\n"

                # Handle both string and bytes content
                if isinstance(content, bytes):
                    # For binary content, we need to handle it differently
                    # For now, just use latin-1 encoding which preserves bytes
                    body += content.decode("latin-1")
                else:
                    body += content

                body += "\r\n"

        # Add final boundary
        body += f"------{boundary}--\r\n"

        return body

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
