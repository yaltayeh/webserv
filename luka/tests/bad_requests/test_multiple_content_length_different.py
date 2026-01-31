"""
Test: Multiple Content-Length headers with different values.
Expected: Should fail per RFC 7230 - conflicting Content-Length headers.
"""

from luka.base_raw_request_test import RawRequestTest


class MultipleContentLengthDifferentTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_multiple_content_length_different"

    def get_description(self) -> str:
        return "Multiple Content-Length headers with different values - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\nContent-Length: 20\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 for conflicting Content-Length headers"
        )
        print("✓ Multiple different Content-Length values rejected")
