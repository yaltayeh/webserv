"""
Test: Request line with only two parts (method and URI, missing HTTP version).
Expected: Should fail - HTTP version is required.
"""

from luka.base_raw_request_test import RawRequestTest


class RequestLineTwoPartsTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_request_line_two_parts"

    def get_description(self) -> str:
        return (
            "Request line with method and URI only (missing HTTP version) - should fail"
        )

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Request with method and URI only
        request = b"GET /index.html\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for request line missing HTTP version"
        )
        print("✓ Request line with two parts rejected")
