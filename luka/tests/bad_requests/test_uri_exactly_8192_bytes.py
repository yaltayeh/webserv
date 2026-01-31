"""
Test: URI exactly at 8192 byte limit.
Expected: Should pass - 8192 is the max URI length (Parser.cpp:69-73).
"""

from luka.base_raw_request_test import RawRequestTest


class UriExactly8192BytesTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_uri_exactly_8192_bytes"

    def get_description(self) -> str:
        return "URI exactly at 8192 byte limit - should pass"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Create URI exactly 8192 bytes
        # Format: "/?" + "a" * (8192 - len("/?"))
        uri_content = "a" * (
            8192 - 2
        )  # -2 for "/?", but we need to account for full request line
        # Full request line: "GET /<uri> HTTP/1.1"
        # So URI length: 8192 - len("GET ") - len(" HTTP/1.1") = 8192 - 4 - 9 = 8179
        # But Parser checks URI itself, not the whole line, so we make URI exactly 8192
        uri = "/" + "a" * 8191
        request = f"GET {uri} HTTP/1.1\r\nHost: localhost\r\n\r\n".encode()

        response = self.send_raw_request(request, wait_time=3)

        # Should accept (404 or 200), not 414
        assert b"414" not in response, (
            "URI at exactly 8192 bytes should be accepted, not rejected with 414"
        )
        print("✓ URI at 8192 bytes accepted")
