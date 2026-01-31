"""
Test: HTTP version in lowercase (http/1.1).
Expected: Should fail - HTTP version is case-sensitive.
"""

from luka.base_raw_request_test import RawRequestTest


class HttpVersionLowercaseTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_http_version_lowercase"

    def get_description(self) -> str:
        return "HTTP version lowercase (http/1.1) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / http/1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"505" in response or b"Bad Request" in response, (
            "Expected error for lowercase HTTP version"
        )
        print("✓ Lowercase HTTP version rejected")
