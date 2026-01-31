"""
Test: Body without Content-Length header.
Expected: Should fail - body requires Content-Length for non-chunked encoding.
"""

from luka.base_raw_request_test import RawRequestTest


class BodyWithoutContentLengthTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_body_without_content_length"

    def get_description(self) -> str:
        return "Body without Content-Length - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # POST with body but no Content-Length
        request = b"POST / HTTP/1.1\r\nHost: localhost\r\n\r\nSome body data"
        response = self.send_raw_request(request)

        assert (
            b"400" in response or b"411" in response or b"Length Required" in response
        ), "Expected 411 Length Required or 400 for body without Content-Length"
        print("✓ Body without Content-Length rejected")
