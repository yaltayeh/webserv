"""
Test: Transfer-Encoding: chunked header.
Expected: Should fail with 501 Not Implemented - webserv doesn't support chunked encoding.
"""

from luka.base_raw_request_test import RawRequestTest


class TransferEncodingChunkedTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_transfer_encoding_chunked"

    def get_description(self) -> str:
        return "Transfer-Encoding: chunked - should return 501"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        request = (
            b"POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n"
        )
        response = self.send_raw_request(request)

        assert (
            b"501" in response or b"Not Implemented" in response or b"400" in response
        ), "Expected 501 Not Implemented or 400 for chunked encoding"
        print("✓ Transfer-Encoding: chunked rejected")
