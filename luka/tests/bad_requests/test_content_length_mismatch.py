"""Test POST with Content-Length but no body."""

import socket
import time
from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class ContentLengthMismatchTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_content_length_mismatch"

    def get_description(self) -> str:
        return "Tests POST with Content-Length but no body."

    def setup(self):
        self.create_html_file("index.html", HTMLBuilder.index_page("Test"))

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
        # Verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")

        # POST with Content-Length but no body
        # Server should wait for the body that never arrives
        # Test with short wait - server won't respond since body isn't complete
        response = self.send_raw_request(
            "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\n",
            wait_time=1,  # Wait only 1 second
        )

        # Server should not send a response yet (still waiting for 10 bytes of body)
        # So we expect either empty response or connection timeout
        if len(response) == 0:
            print("✓ Server correctly waits for body (no premature response)")
        else:
            # If we got an unexpected response, that's an error
            raise AssertionError(
                f"Server sent response before receiving complete body: {response[:100]}"
            )
