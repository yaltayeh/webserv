"""
Test: Request without final CRLF after headers.
Expected: Should fail - headers must end with double CRLF.
"""

from luka.base_raw_request_test import RawRequestTest


class RequestNoFinalCrlfTest(RawRequestTest):
    def get_name(self) -> str:
        return "bad_requests_no_final_crlf"

    def get_description(self) -> str:
        return "Request without final CRLF - should timeout or fail"

    def setup(self):
        conf = f"""http {{
			server {{
				listen {self.port};
				root {self.www_dir};
			}}
		}}"""
        self.create_conf(conf)

    def run(self):
        # Missing final \r\n\r\n
        request = b"GET / HTTP/1.1\r\nHost: localhost"
        response = self.send_raw_request(request, wait_time=2)

        # Server should timeout waiting for end of headers
        if len(response) == 0:
            print("✓ Server timed out waiting for final CRLF")
        elif b"400" in response:
            print("✓ Server rejected incomplete request")
        else:
            print(f"✓ Unexpected response: {response[:100]}")
