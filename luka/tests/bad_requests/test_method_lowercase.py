"""
Test: HTTP method in lowercase (get).
Expected: Should fail - HTTP methods are case-sensitive (must be uppercase).
"""

from luka.base_raw_request_test import RawRequestTest


class MethodLowercaseTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_method_lowercase"

    def get_description(self) -> str:
        return "Lowercase HTTP method (get) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"get / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"501" in response or b"Bad Request" in response, (
            "Expected error for lowercase HTTP method"
        )
        print("✓ Lowercase method rejected")
