"""
Test: Null byte in header value.
Expected: Should fail - null bytes not allowed in headers.
"""

from luka.base_raw_request_test import RawRequestTest


class NullByteInHeaderValueTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_null_byte_in_header_value"

    def get_description(self) -> str:
        return "Null byte in header value - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Header with null byte
        request = (
            b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: value\x00data\r\n\r\n"
        )
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response or len(response) == 0, (
            "Expected 400 Bad Request for null byte in header"
        )
        print("✓ Null byte in header rejected")
