"""
Test: HTTP method with mixed case (GeT).
Expected: Should fail - HTTP methods are case-sensitive.
"""

from luka.base_raw_request_test import RawRequestTest


class MethodMixedCaseTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_method_mixed_case"

    def get_description(self) -> str:
        return "Mixed case HTTP method (GeT) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GeT / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"501" in response or b"Bad Request" in response, (
            "Expected error for mixed case HTTP method"
        )
        print("✓ Mixed case method rejected")
