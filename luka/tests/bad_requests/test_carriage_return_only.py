"""
Test: Lines with carriage return only (\\r without \\n).
Expected: Should fail - HTTP requires CRLF (\\r\\n) line endings.
"""

from luka.base_raw_request_test import RawRequestTest


class CarriageReturnOnlyTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_carriage_return_only"

    def get_description(self) -> str:
        return "Lines with \\r only (no \\n) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Use \r only instead of \r\n
        request = b"GET / HTTP/1.1\rHost: localhost\r\r"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response or len(response) == 0, (
            "Expected 400 or no response for \\r only line endings"
        )
        print("✓ Carriage return only rejected")
