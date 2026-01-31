"""
Test: Tab character in header value.
Expected: Should pass - tabs are allowed in header values per RFC 7230.
"""

from luka.base_raw_request_test import RawRequestTest


class TabInHeaderValueTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_tab_in_header_value"

    def get_description(self) -> str:
        return "Tab in header value - should pass"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Header with tab character
        request = (
            b"GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: value\twith\ttabs\r\n\r\n"
        )
        response = self.send_raw_request(request)

        # Should accept (200 or 404), not 400
        assert b"400" not in response, (
            "Tabs in header values should be allowed per RFC 7230"
        )
        print("✓ Tab in header value accepted")
