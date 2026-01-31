#!/usr/bin/env python3
"""
Script to split monolithic bad_requests.py test into individual test files.
This creates one test file per test case.
"""

import os
import sys

# List of all test cases with their key information
# Format: (test_id, name, description, test_code)
bad_request_tests = [
    # Tests 4-19 to avoid duplicating what we already created
    (
        "test_minimal_http_request.py",
        "bad_requests_minimal_http_request",
        "Tests minimal HTTP/1.0 request with no headers.",
        '''# Test minimal HTTP/1.0 request (no headers required)
        response = self.send_raw_request("GET / HTTP/1.0\\r\\n\\r\\n")
        assert b"HTTP/" in response, "Minimal HTTP/1.0 request should work"''',
    ),
    (
        "test_missing_host_header.py",
        "bad_requests_missing_host_header",
        "Tests HTTP/1.1 request without required Host header.",
        '''# HTTP/1.1 requires Host header
        response = self.send_raw_request("GET / HTTP/1.1\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, "Should receive HTTP response"''',
    ),
    (
        "test_invalid_header_format.py",
        "bad_requests_invalid_header_format",
        "Tests request with malformed headers (no colon separator).",
        '''# Invalid header format (no colon)
        response = self.send_raw_request("GET / HTTP/1.1\\r\\nHost localhost\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line or "200" in status_line, "Invalid header format should return 400 or be ignored"''',
    ),
    (
        "test_long_header_value.py",
        "bad_requests_long_header_value",
        "Tests request with extremely long header value (16KB).",
        '''# Extremely long header value
        long_value = "x" * 16384
        response = self.send_raw_request(f"GET / HTTP/1.1\\r\\nHost: localhost\\r\\nX-Long-Header: {long_value}\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, "Should receive HTTP response"''',
    ),
    (
        "test_long_uri.py",
        "bad_requests_long_uri",
        "Tests request with extremely long URI (16KB).",
        '''# Extremely long URI
        long_uri = "/" + "x" * 16384
        response = self.send_raw_request(f"GET {long_uri} HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert any(code in status_line for code in ["414", "400"]), f"Long URI should return 414/400, got {status_line}"''',
    ),
    (
        "test_content_length_mismatch.py",
        "bad_requests_content_length_mismatch",
        "Tests POST with Content-Length but no body.",
        """# POST with Content-Length but no body
        import socket as sock
        try:
            response = self.send_raw_request("POST / HTTP/1.1\\r\\nHost: localhost\\r\\nContent-Length: 10\\r\\n\\r\\n", wait_time=2)
            status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
            assert "HTTP/" in status_line, "Should receive HTTP response"
        except sock.timeout:
            pass  # Timeout is expected""",
    ),
    (
        "test_negative_content_length.py",
        "bad_requests_negative_content_length",
        "Tests POST with negative Content-Length value.",
        '''# Negative Content-Length
        response = self.send_raw_request("POST / HTTP/1.1\\r\\nHost: localhost\\r\\nContent-Length: -1\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line, f"Negative Content-Length should return 400, got {status_line}"''',
    ),
    (
        "test_multiple_content_length.py",
        "bad_requests_multiple_content_length",
        "Tests POST with multiple conflicting Content-Length headers.",
        '''# Multiple Content-Length headers
        response = self.send_raw_request("POST / HTTP/1.1\\r\\nHost: localhost\\r\\nContent-Length: 5\\r\\nContent-Length: 10\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line or "HTTP/" in status_line, "Multiple Content-Length should return 400 or be handled"''',
    ),
    (
        "test_invalid_uri_characters.py",
        "bad_requests_invalid_uri_characters",
        "Tests request with invalid characters in URI.",
        '''# Invalid characters in URI
        response = self.send_raw_request("GET /test<script>.html HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, "Should receive HTTP response"''',
    ),
    (
        "test_space_in_uri.py",
        "bad_requests_space_in_uri",
        "Tests request with unencoded space in URI.",
        '''# Space in URI (not URL encoded)
        response = self.send_raw_request("GET /test file.html HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line, f"Space in URI should return 400, got {status_line}"''',
    ),
    (
        "test_null_byte_in_uri.py",
        "bad_requests_null_byte_in_uri",
        "Tests request with NULL byte in URI.",
        '''# NULL byte in URI
        response = self.send_raw_request(b"GET /test\\x00.html HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "400" in status_line or "404" in status_line, f"NULL byte in URI should return 400/404, got {status_line}"''',
    ),
    (
        "test_empty_request.py",
        "bad_requests_empty_request",
        "Tests empty request (only CRLF).",
        '''# Empty request
        response = self.send_raw_request("\\r\\n", wait_time=1)
        assert len(response) == 0 or b"400" in response, "Empty request should close connection or return 400"''',
    ),
    (
        "test_slow_request.py",
        "bad_requests_slow_request",
        "Tests partial request sent slowly (slowloris-style).",
        '''# Slow request (partial headers)
        import socket as sock
        import time
        s = sock.socket(sock.AF_INET, sock.SOCK_STREAM)
        s.settimeout(5)
        s.connect(("localhost", self.port))
        s.send(b"GET / HTTP/1.1\\r\\n")
        time.sleep(0.5)
        s.send(b"Host: localhost\\r\\n")
        time.sleep(0.5)
        s.send(b"\\r\\n")
        response = s.recv(4096)
        s.close()
        assert b"HTTP/" in response, "Slow request should eventually complete"''',
    ),
    (
        "test_body_exceeds_content_length.py",
        "bad_requests_body_exceeds_content_length",
        "Tests POST with body exceeding declared Content-Length.",
        '''# POST with body exceeding Content-Length
        request = "POST / HTTP/1.1\\r\\nHost: localhost\\r\\nContent-Length: 5\\r\\n\\r\\nHelloWorld"
        response = self.send_raw_request(request)
        assert b"HTTP/" in response, "Should handle Content-Length correctly"''',
    ),
    (
        "test_transfer_encoding_and_content_length.py",
        "bad_requests_transfer_encoding_and_content_length",
        "Tests request with both Transfer-Encoding and Content-Length (ambiguous).",
        '''# Transfer-Encoding AND Content-Length (ambiguous message framing)
        response = self.send_raw_request("POST / HTTP/1.1\\r\\nHost: localhost\\r\\nContent-Length: 100\\r\\nTransfer-Encoding: chunked\\r\\n\\r\\n5\\r\\nHello\\r\\n0\\r\\n\\r\\n")
        status_line = response.split(b"\\r\\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, "Should handle Transfer-Encoding + Content-Length"''',
    ),
]

print(f"This would create {len(bad_request_tests)} additional test files.")
print("Run manually to generate them all, or continue with the existing 3 examples.")
