"""
Test: Header without key (colon at start).
Expected: Should fail - headers must have a key.
"""

from luka.base_raw_request_test import RawRequestTest


class HeaderEmptyKeyTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_header_empty_key"

    def get_description(self) -> str:
        return "Header without key (: value) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / HTTP/1.1\r\n: value\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response, (
            "Expected 400 Bad Request for header without key"
        )
        print("✓ Header without key rejected")
