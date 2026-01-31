"""
Test: Mixed line endings (both \\r\\n and \\n).
Expected: Behavior test - some servers accept mixed endings.
"""

from luka.base_raw_request_test import RawRequestTest


class MixedLineEndingsTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_mixed_line_endings"

    def get_description(self) -> str:
        return "Mixed line endings - behavior test"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Mix \r\n and \n
        request = b"GET / HTTP/1.1\r\nHost: localhost\n\r\n"
        response = self.send_raw_request(request)

        if b"400" in response:
            print("✓ Server rejects mixed line endings")
        elif b"200" in response or b"404" in response:
            print("✓ Server accepts mixed line endings")
        else:
            print(f"✓ Unexpected response: {response[:100]}")
