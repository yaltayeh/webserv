"""
Test: URI exceeding 8192 byte limit (8193 bytes).
Expected: Should fail with 414 URI Too Long.
"""

from luka.base_raw_request_test import RawRequestTest


class Uri8193BytesTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_uri_8193_bytes"

    def get_description(self) -> str:
        return "URI exceeding 8192 byte limit - should return 414"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Create URI with 8193 bytes (over limit)
        uri = "/" + "a" * 8192
        request = f"GET {uri} HTTP/1.1\r\nHost: localhost\r\n\r\n".encode()

        response = self.send_raw_request(request, wait_time=3)

        assert b"414" in response or b"URI Too Long" in response, (
            "Expected 414 URI Too Long for URI exceeding 8192 bytes"
        )
        print("✓ URI over 8192 bytes rejected with 414")
