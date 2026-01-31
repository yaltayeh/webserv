"""
Test: Header with empty value (key but no value).
Expected: Should pass - empty header values are allowed.
"""

from luka.base_raw_request_test import RawRequestTest


class HeaderEmptyValueTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_header_empty_value"

    def get_description(self) -> str:
        return "Header with empty value - should pass"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom:\r\n\r\n"
        response = self.send_raw_request(request)

        # Should accept (200 or 404), not 400
        assert b"400" not in response, "Empty header values should be allowed"
        print("✓ Header with empty value accepted")
