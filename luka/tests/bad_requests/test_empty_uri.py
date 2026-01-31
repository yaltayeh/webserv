"""
Test: Empty URI in request line.
Expected: Should fail - URI cannot be empty.
"""

from luka.base_raw_request_test import RawRequestTest


class EmptyUriTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_empty_uri"

    def get_description(self) -> str:
        return "Empty URI - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Two spaces between method and version (empty URI)
        request = b"GET  HTTP/1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for empty URI"
        )
        print("✓ Empty URI rejected")
