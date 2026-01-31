"""
Test: Both Transfer-Encoding and Content-Length headers present.
Expected: Should fail per RFC 7230 - cannot have both.
"""

from luka.base_raw_request_test import RawRequestTest


class TransferEncodingWithContentLengthTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_transfer_encoding_with_content_length"

    def get_description(self) -> str:
        return "Both Transfer-Encoding and Content-Length - should fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = b"POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\nContent-Length: 10\r\n\r\n"
        response = self.send_raw_request(request)

        assert b"400" in response or b"501" in response or b"Bad Request" in response, (
            "Expected error for both TE and CL headers"
        )
        print("✓ Transfer-Encoding with Content-Length rejected")
