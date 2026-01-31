"""
Test: Content-Length with decimal value (10.5).
Expected: Should fail - Content-Length must be an integer.
"""

from luka.base_raw_request_test import RawRequestTest


class ContentLengthDecimalTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_content_length_decimal"

    def get_description(self) -> str:
        return "Content-Length with decimal - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10.5\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for decimal Content-Length"
        )
        print("✓ Decimal Content-Length rejected")
