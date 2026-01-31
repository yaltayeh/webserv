"""
Test: Body at exactly client_max_body_size limit.
Expected: Should pass - at limit should be acceptable.
"""

from luka.base_raw_request_test import RawRequestTest


class BodyExactlyAtMaxTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_body_exactly_at_max"

    def get_description(self) -> str:
        return "Body at exactly client_max_body_size - should pass"

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
        # Send exactly 100 bytes (at limit)
        body = b"a" * 100
        request = (
            f"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 100\r\n\r\n".encode()
            + body
        )
        response = self.send_raw_request(request)

        assert b"413" not in response, (
            "Body at exact limit should be accepted, not rejected with 413"
        )
        print("✓ Body at exact limit accepted")
