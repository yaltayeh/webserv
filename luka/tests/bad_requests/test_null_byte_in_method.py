"""
Test: Null byte in method.
Expected: Should fail - null bytes not allowed in request line.
"""

from luka.base_raw_request_test import RawRequestTest


class NullByteInMethodTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_null_byte_in_method"

    def get_description(self) -> str:
        return "Null byte in HTTP method - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Method with null byte
        request = b"GET\x00 / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response or len(response) == 0, (
            "Expected 400 Bad Request for null byte in method"
        )
        print("✓ Null byte in method rejected")
