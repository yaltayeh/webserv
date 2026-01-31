"""
Test: URI with fragment identifier (#fragment).
Expected: Behavior test - fragments should not be sent by client per RFC 7230.
"""

from luka.base_raw_request_test import RawRequestTest


class UriWithFragmentTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_uri_with_fragment"

    def get_description(self) -> str:
        return "URI with fragment identifier - behavior test"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET /index.html#section HTTP/1.1\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        # Server may accept it or reject it - just document the behavior
        if b"200" in response or b"404" in response:
            print("✓ Server accepts URI with fragment (treats as part of path)")
        elif b"400" in response:
            print("✓ Server rejects URI with fragment")
        else:
            print(f"✓ Unexpected response: {response[:100]}")
