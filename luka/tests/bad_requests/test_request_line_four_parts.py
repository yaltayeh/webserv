"""
Test: Request line with four parts (extra trailing data).
Expected: Should fail - request line should have exactly 3 parts.
"""

from luka.base_raw_request_test import RawRequestTest


class RequestLineFourPartsTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_request_line_four_parts"

    def get_description(self) -> str:
        return "Request line with four parts (extra data) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Request with four parts
        request = b"GET / HTTP/1.1 extra\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for request line with extra parts"
        )
        print("✓ Request line with four parts rejected")
