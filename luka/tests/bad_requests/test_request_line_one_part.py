"""
Test: Request line with only one part (method only).
Expected: Should fail - request line requires 3 parts (method, URI, version).
"""

from luka.base_raw_request_test import RawRequestTest


class RequestLineOnePartTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_request_line_one_part"

    def get_description(self) -> str:
        return "Request line with only method (no URI or version) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Request with only method
        request = b"GET\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for incomplete request line"
        )
        print("✓ Request line with one part rejected")
