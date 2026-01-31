"""
Test: Header with colon in value (Host: localhost:8080).
Expected: Should pass - colons are allowed in header values.
"""

from luka.base_raw_request_test import RawRequestTest


class HeaderColonInValueTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_header_colon_in_value"

    def get_description(self) -> str:
        return "Header with colon in value - should pass"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = self.send_raw_request(request)

        # Should accept (200 or 404), not 400
        assert b"400" not in response, "Colons in header values should be allowed"
        print("✓ Colon in header value accepted")
