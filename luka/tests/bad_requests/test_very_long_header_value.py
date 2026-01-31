"""
Test: Very long header value (over 8192 bytes).
Expected: Should fail - header values have size limits.
"""

from luka.base_raw_request_test import RawRequestTest


class VeryLongHeaderValueTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_very_long_header_value"

    def get_description(self) -> str:
        return "Very long header value (>8192 bytes) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Create header value with 10000 bytes
        long_value = "a" * 10000
        request = f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Long: {long_value}\r\n\r\n".encode()
        response = self.send_raw_request(request, wait_time=3)

        assert (
            b"400" in response
            or b"431" in response
            or b"Request Header Fields Too Large" in response
        ), "Expected 400 or 431 for very long header value"
        print("✓ Very long header value rejected")
