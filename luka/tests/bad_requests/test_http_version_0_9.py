"""
Test: HTTP/0.9 version (unsupported).
Expected: Should fail - Parser.cpp:76-80 only supports HTTP/1.0 and HTTP/1.1.
"""

from luka.base_raw_request_test import RawRequestTest


class HttpVersion09Test(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_http_version_0_9"

    def get_description(self) -> str:
        return "HTTP/0.9 version - should fail (only 1.0/1.1 supported)"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"GET / HTTP/0.9\r\nHost: localhost\r\n\r\n"
        response = self.send_raw_request(request)

        assert (
            b"505" in response or b"400" in response or b"HTTP Version" in response
        ), "Expected 505 HTTP Version Not Supported or 400 for HTTP/0.9"
        print("✓ HTTP/0.9 rejected")
