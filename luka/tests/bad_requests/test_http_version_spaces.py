"""
Test: HTTP version with spaces (HTTP / 1.1).
Expected: Should fail - malformed version string.
"""

from luka.base_raw_request_test import RawRequestTest


class HttpVersionSpacesTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_http_version_spaces"

    def get_description(self) -> str:
        return "HTTP version with spaces - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / HTTP / 1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for malformed HTTP version"
        )
        print("✓ HTTP version with spaces rejected")
