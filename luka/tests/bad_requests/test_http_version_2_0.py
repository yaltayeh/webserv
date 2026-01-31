"""
Test: HTTP/2.0 version (unsupported).
Expected: Should fail with 505 HTTP Version Not Supported.
"""

from luka.base_raw_request_test import RawRequestTest


class HttpVersion20Test(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_http_version_2_0"

    def get_description(self) -> str:
        return "HTTP/2.0 version - should return 505 (not supported)"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / HTTP/2.0\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"505" in response or b"HTTP Version Not Supported" in response, (
            "Expected 505 HTTP Version Not Supported for HTTP/2.0"
        )
        print("✓ HTTP/2.0 rejected with 505")
