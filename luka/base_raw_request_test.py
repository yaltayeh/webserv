"""
Base class for tests that need to send raw HTTP requests.
"""

import socket
from luka.base_test import BaseTest


class RawRequestTest(BaseTest):
    """
    Extended BaseTest that adds support for sending raw HTTP requests.
    Useful for testing malformed requests and edge cases.
    """

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
