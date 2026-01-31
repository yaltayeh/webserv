"""
Test: Header with whitespace before colon (RFC violation).
Expected: Should fail - whitespace before colon is not allowed per RFC 7230.
"""

from luka.base_raw_request_test import RawRequestTest


class HeaderWhitespaceBeforeColonTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_header_whitespace_before_colon"

    def get_description(self) -> str:
        return "Header with whitespace before colon - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Space before colon is RFC violation
        request = b"GET / HTTP/1.1\r\nHost : localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for whitespace before colon"
        )
        print("✓ Whitespace before colon rejected")
