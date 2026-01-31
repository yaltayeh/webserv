"""
Test: Content-Length in hexadecimal (0x10).
Expected: Should fail - Content-Length must be decimal, not hex.
"""

from luka.base_raw_request_test import RawRequestTest


class ContentLengthHexTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_content_length_hex"

    def get_description(self) -> str:
        return "Content-Length in hexadecimal - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0x10\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for hex Content-Length"
        )
        print("✓ Hex Content-Length rejected")
