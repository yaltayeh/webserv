"""
Test: Content-Length: 0 but body is present.
Expected: Should fail - body doesn't match Content-Length.
"""

from luka.base_raw_request_test import RawRequestTest


class ContentLengthZeroWithBodyTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_content_length_zero_with_body"

    def get_description(self) -> str:
        return "Content-Length: 0 with body present - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\nUnexpected body"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 for body when Content-Length is 0"
        )
        print("✓ CL:0 with body rejected")
