"""
Test: Lines with line feed only (\\n without \\r).
Expected: Should fail - HTTP requires CRLF (\\r\\n), not LF alone.
"""

from luka.base_raw_request_test import RawRequestTest


class LineFeedOnlyTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_line_feed_only"

    def get_description(self) -> str:
        return "Lines with \\n only (no \\r) - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Use \n only instead of \r\n
        request = b"GET / HTTP/1.1\nHost: localhost\n\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"Bad Request" in response or len(response) == 0, (
            "Expected 400 or no response for \\n only line endings"
        )
        print("✓ Line feed only rejected")
