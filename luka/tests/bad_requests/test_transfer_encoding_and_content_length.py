"""Test request with both Transfer-Encoding and Content-Length (ambiguous)."""

from luka.base_raw_request_test import RawRequestTest
from luka.utilities.test_helpers import HTMLBuilder


class TransferEncodingAndContentLengthTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_transfer_encoding_and_content_length"

    def get_description(self) -> str:
        return (
            "Tests request with both Transfer-Encoding and Content-Length (ambiguous)."
        )

    def setup(self):
        self.create_html_file("index.html", HTMLBuilder.index_page("Test"))

        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should be running")

        # Transfer-Encoding AND Content-Length (ambiguous message framing)
        response = self.send_raw_request(
            "POST / HTTP/1.1\r\nHost: localhost\r\n"
            + "Content-Length: 100\r\nTransfer-Encoding: chunked\r\n\r\n"
            + "5\r\nHello\r\n0\r\n\r\n"
        )
        status_line = response.split(b"\r\n")[0].decode("utf-8", errors="ignore")
        assert "HTTP/" in status_line, (
            "Should handle Transfer-Encoding + Content-Length"
        )
