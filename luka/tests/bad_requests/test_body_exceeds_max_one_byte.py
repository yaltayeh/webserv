"""
Test: Body exceeds client_max_body_size by one byte.
Expected: Should fail with 413 Payload Too Large.
"""

from luka.base_raw_request_test import RawRequestTest


class BodyExceedsOneByteTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_body_exceeds_one_byte"

    def get_description(self) -> str:
        return "Body exceeds client_max_body_size - should return 413"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
				client_max_body_size 100;
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Send 101 bytes (exceeds limit of 100)
        body = b"a" * 101
        request = (
            f"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 101\r\n\r\n".encode()
            + body
        )
        response = self.send_raw_request(request)

        assert (
            b"413" in response
            or b"Payload Too Large" in response
            or b"Request Entity Too Large" in response
        ), "Expected 413 Payload Too Large for body exceeding limit"
        print("✓ Body exceeding limit rejected with 413")
