"""
Test: Content-Length with leading zeros (00100).
Expected: Should pass - leading zeros are technically valid.
"""

from luka.base_raw_request_test import RawRequestTest


class ContentLengthLeadingZerosTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_content_length_leading_zeros"

    def get_description(self) -> str:
        return "Content-Length with leading zeros - behavior test"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Create a 100-byte body
        body = b"a" * 100
        request = (
            b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 00100\r\n\r\n"
            + body
        )
        response = self.send_raw_request(request)

        # Most servers accept leading zeros
        if b"400" in response:
            print("✓ Server rejects Content-Length with leading zeros")
        else:
            print("✓ Server accepts Content-Length with leading zeros")
