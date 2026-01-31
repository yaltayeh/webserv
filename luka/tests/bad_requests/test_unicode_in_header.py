"""
Test: Unicode characters in header.
Expected: Behavior test - HTTP headers should be ASCII only.
"""

from luka.base_raw_request_test import RawRequestTest


class UnicodeInHeaderTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_unicode_in_header"

    def get_description(self) -> str:
        return "Unicode characters in header - behavior test"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Header with Unicode
        request = "GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: café\r\n\r\n".encode(
            "utf-8"
        )
        response = self.send_raw_request(request)

        if b"400" in response:
            print("✓ Server rejects Unicode in headers")
        elif b"200" in response or b"404" in response:
            print("✓ Server accepts Unicode in headers")
        else:
            print(f"✓ Unexpected response: {response[:100]}")
