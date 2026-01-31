"""
Test: Multiple Content-Length headers with same value.
Expected: Behavior test - RFC 7230 says same value is technically valid.
"""

from luka.base_raw_request_test import RawRequestTest


class MultipleContentLengthSameTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_multiple_content_length_same"

    def get_description(self) -> str:
        return "Multiple Content-Length headers with same value - behavior test"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        body = b"a" * 10
        request = (
            b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\nContent-Length: 10\r\n\r\n"
            + body
        )
        response = self.send_raw_request(request)

        # Some servers accept, some reject
        if b"400" in response:
            print("✓ Server rejects duplicate Content-Length headers")
        else:
            print("✓ Server accepts duplicate Content-Length with same value")
