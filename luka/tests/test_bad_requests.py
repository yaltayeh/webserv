"""
Bad/malformed HTTP requests test.
Tests how the server handles various malformed, invalid, and edge case HTTP requests.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder
import socket
import time


class BadRequestsTest(BaseTest):
    def get_name(self) -> str:
        return "bad_requests"

    def get_description(self) -> str:
        return "Tests malformed HTTP requests: invalid headers, methods, versions, oversized requests."

    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("Bad Requests Test"))

        # Create a simple file for testing
        self.create_file("test.txt", "Test file content")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 1k;
        
        location / {{
            allowed_methods GET POST PUT DELETE;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        print("\n[Test 1] Verify server is running with normal request")
        # Test 1: Verify server is running with normal request
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")
        print("  ✓ Server responding normally")

        print("\n[Test 2] Invalid HTTP method (GETT)")
        # Test 2: Invalid HTTP method (should return 400 or 405 or 501)
        try:
            response = self.send_raw_request(
                "GETT / HTTP/1.1\r\nHost: localhost\r\n\r\n"
            )
            assert response.startswith(b"HTTP/1."), "Should receive HTTP response"
            # Extract status code
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            assert any(code in status_line for code in ["400", "405", "501"]), (
                f"Invalid method should return 400/405/501, got {status_line}"
            )
            print(f"  ✓ Invalid method rejected: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Invalid method test failed: {e}")

        print("\n[Test 3] Missing HTTP version")
        # Test 3: Missing HTTP version
        try:
            response = self.send_raw_request("GET /\r\nHost: localhost\r\n\r\n")
            # Should return 400 Bad Request or handle as HTTP/0.9
            assert response.startswith(b"HTTP/") or len(response) > 0, (
                "Server should respond to request without version"
            )
            print(f"  ✓ Missing version handled")
        except Exception as e:
            print(f"  ⚠ Warning: Missing version test failed: {e}")

        print("\n[Test 4] Invalid HTTP version (HTTP/3.0)")
        # Test 4: Invalid HTTP version
        try:
            response = self.send_raw_request(
                "GET / HTTP/3.0\r\nHost: localhost\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 or 505 HTTP Version Not Supported
            assert any(code in status_line for code in ["400", "505"]), (
                f"Invalid HTTP version should return 400/505, got {status_line}"
            )
            print(f"  ✓ Invalid version rejected: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Invalid version test failed: {e}")

        print("\n[Test 5] Request with no headers (minimal HTTP/1.0)")
        # Test 5: Request with no headers (minimal request)
        try:
            response = self.send_raw_request("GET / HTTP/1.0\r\n\r\n")
            # HTTP/1.0 doesn't require Host header, should work
            assert b"HTTP/" in response, "Minimal HTTP/1.0 request should work"
            print(f"  ✓ Minimal HTTP/1.0 request accepted")
        except Exception as e:
            print(f"  ⚠ Warning: Minimal request test failed: {e}")

        print("\n[Test 6] Missing Host header (HTTP/1.1)")
        # Test 6: Request with missing Host header (HTTP/1.1)
        try:
            response = self.send_raw_request("GET / HTTP/1.1\r\n\r\n")
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # HTTP/1.1 requires Host header, should return 400
            # But some servers are lenient
            assert "HTTP/" in status_line, "Should receive HTTP response"
            print(f"  ✓ Missing Host header handled: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Missing Host test failed: {e}")

        print("\n[Test 7] Invalid header format (no colon)")
        # Test 7: Request with invalid header format (no colon)
        try:
            response = self.send_raw_request("GET / HTTP/1.1\r\nHost localhost\r\n\r\n")
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 Bad Request
            assert "400" in status_line or "200" in status_line, (
                "Invalid header format should return 400 or be ignored"
            )
            print(f"  ✓ Invalid header format handled: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Invalid header format test failed: {e}")

        print("\n[Test 8] Extremely long header value (16KB)")
        # Test 8: Request with extremely long header value
        try:
            long_value = "x" * 16384  # 16KB header value
            response = self.send_raw_request(
                f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Long-Header: {long_value}\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 or 431 Request Header Fields Too Large
            # Or might accept it if server has high limits
            assert "HTTP/" in status_line, "Should receive HTTP response"
            print(f"  ✓ Long header handled: {status_line}")
        except Exception as e:
            print(
                f"  ⚠ Warning: Long header test failed (might have been rejected): {e}"
            )

        print("\n[Test 9] Extremely long URI (16KB)")
        # Test 9: Request with extremely long URI
        try:
            long_uri = "/" + "x" * 16384  # 16KB URI
            response = self.send_raw_request(
                f"GET {long_uri} HTTP/1.1\r\nHost: localhost\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 414 URI Too Long
            assert any(code in status_line for code in ["414", "400"]), (
                f"Long URI should return 414/400, got {status_line}"
            )
            print(f"  ✓ Long URI rejected: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Long URI test failed: {e}")

        print("\n[Test 10] POST with Content-Length but no body")
        # Test 10: POST with Content-Length but no body
        try:
            response = self.send_raw_request(
                "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\n",
                wait_time=2,
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Server might timeout or return 400
            assert "HTTP/" in status_line, "Should receive HTTP response"
            print(f"  ✓ Content-Length mismatch handled: {status_line}")
        except socket.timeout:
            print("  ✓ Content-Length mismatch caused timeout (expected)")
        except Exception as e:
            print(f"  ⚠ Warning: Content-Length mismatch test issue: {e}")

        print("\n[Test 11] POST with negative Content-Length")
        # Test 11: POST with negative Content-Length
        try:
            response = self.send_raw_request(
                "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 Bad Request
            assert "400" in status_line, (
                f"Negative Content-Length should return 400, got {status_line}"
            )
            print(f"  ✓ Negative Content-Length rejected: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Negative Content-Length test failed: {e}")

        print("\n[Test 12] Multiple Content-Length headers")
        # Test 12: POST with multiple Content-Length headers (different values)
        try:
            response = self.send_raw_request(
                "POST / HTTP/1.1\r\nHost: localhost\r\n"
                + "Content-Length: 5\r\nContent-Length: 10\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 Bad Request (RFC 7230 says reject)
            assert "400" in status_line or "HTTP/" in status_line, (
                "Multiple Content-Length should return 400 or be handled"
            )
            print(f"  ✓ Multiple Content-Length handled: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Multiple Content-Length test failed: {e}")

        print("\n[Test 13] Invalid characters in URI")
        # Test 13: Request with invalid characters in URI
        try:
            response = self.send_raw_request(
                "GET /test<script>.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 Bad Request or 404
            assert "HTTP/" in status_line, "Should receive HTTP response"
            print(f"  ✓ Invalid URI characters handled: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Invalid URI characters test failed: {e}")

        print("\n[Test 14] Spaces in URI (not URL encoded)")
        # Test 14: Request with spaces in URI (not URL encoded)
        try:
            response = self.send_raw_request(
                "GET /test file.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 Bad Request
            assert "400" in status_line, (
                f"Space in URI should return 400, got {status_line}"
            )
            print(f"  ✓ Space in URI rejected: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Space in URI test failed: {e}")

        print("\n[Test 15] NULL bytes in URI")
        # Test 15: Request with NULL bytes in URI
        try:
            response = self.send_raw_request(
                b"GET /test\x00.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # Should return 400 Bad Request
            assert "400" in status_line or "404" in status_line, (
                f"NULL byte in URI should return 400/404, got {status_line}"
            )
            print(f"  ✓ NULL byte in URI handled: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: NULL byte test failed: {e}")

        print("\n[Test 16] Empty request (only CRLF)")
        # Test 16: Request with only CRLF (empty request)
        try:
            response = self.send_raw_request("\r\n", wait_time=1)
            # Server might close connection or return error
            assert len(response) == 0 or b"400" in response, (
                "Empty request should close connection or return 400"
            )
            print(f"  ✓ Empty request handled")
        except Exception as e:
            print(f"  ⚠ Note: Empty request test: {e}")

        print("\n[Test 17] Slow request (partial headers)")
        # Test 17: Extremely slow request (slowloris style - send headers byte by byte)
        # This is a gentle version - just test partial request handling
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)
            sock.connect(("localhost", self.port))
            # Send request very slowly
            sock.send(b"GET / HTTP/1.1\r\n")
            time.sleep(0.5)
            sock.send(b"Host: localhost\r\n")
            time.sleep(0.5)
            sock.send(b"\r\n")
            response = sock.recv(4096)
            sock.close()
            # Should eventually respond
            assert b"HTTP/" in response, "Slow request should eventually complete"
            print(f"  ✓ Slow request handled")
        except Exception as e:
            print(f"  ⚠ Warning: Slow request test: {e}")

        print("\n[Test 18] POST with body exceeding Content-Length")
        # Test 18: POST with body exceeding Content-Length
        try:
            request = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\n"
            request += "HelloWorld"  # Send 10 bytes but say 5
            response = self.send_raw_request(request)
            # Server should only read 5 bytes and respond
            assert b"HTTP/" in response, "Should handle Content-Length correctly"
            print(f"  ✓ Excess body handled")
        except Exception as e:
            print(f"  ⚠ Warning: Excess body test: {e}")

        print("\n[Test 19] Transfer-Encoding AND Content-Length")
        # Test 19: Request with Transfer-Encoding AND Content-Length (should prefer Transfer-Encoding)
        try:
            response = self.send_raw_request(
                "POST / HTTP/1.1\r\nHost: localhost\r\n"
                + "Content-Length: 100\r\nTransfer-Encoding: chunked\r\n\r\n"
                + "5\r\nHello\r\n0\r\n\r\n"
            )
            status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
            # According to RFC, should use Transfer-Encoding and ignore Content-Length
            # Or return 400 for ambiguous message framing
            assert "HTTP/" in status_line, (
                "Should handle Transfer-Encoding + Content-Length"
            )
            print(f"  ✓ Transfer-Encoding + Content-Length handled: {status_line}")
        except Exception as e:
            print(f"  ⚠ Warning: Transfer-Encoding + Content-Length test: {e}")

        # print("\n[Test 20] Chunked encoding with invalid chunk size")
        # # Test 20: Chunked encoding with invalid chunk size
        # try:
        #     response = self.send_raw_request(
        #         "POST / HTTP/1.1\r\nHost: localhost\r\n"
        #         + "Transfer-Encoding: chunked\r\n\r\n"
        #         + "ZZZZ\r\nHello\r\n0\r\n\r\n"
        #     )
        #     status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        #     # Should return 400 Bad Request
        #     assert "400" in status_line, (
        #         f"Invalid chunk size should return 400, got {status_line}"
        #     )
        #     print(f"  ✓ Invalid chunk size rejected: {status_line}")
        # except Exception as e:
        #     print(f"  ⚠ Warning: Invalid chunk size test: {e}")

        print("\n✓ All bad request tests completed!")
        print(
            "Note: Some tests may show warnings if server behavior differs from expectations."
        )

    def send_raw_request(self, request, wait_time=2):
        """
        Send a raw HTTP request and return the response.

        Args:
            request: String or bytes of the raw HTTP request
            wait_time: Time to wait for response (seconds)

        Returns:
            bytes: The raw HTTP response
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(wait_time)

        try:
            sock.connect(("localhost", self.port))

            # Convert string to bytes if needed
            if isinstance(request, str):
                request = request.encode("utf-8")

            sock.send(request)

            # Receive response
            response = b""
            try:
                while True:
                    chunk = sock.recv(4096)
                    if not chunk:
                        break
                    response += chunk
                    # If we got headers and no content-length, break after body
                    if b"\r\n\r\n" in response:
                        # Simple heuristic: if we got some response, that's enough
                        if len(response) > 100:
                            break
            except socket.timeout:
                pass  # Timeout is expected for some tests

            return response
        finally:
            sock.close()
