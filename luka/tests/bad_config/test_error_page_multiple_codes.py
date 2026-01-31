"""
error_page multiple codes test.
Tests that error_page with multiple error codes is accepted.
"""

from luka.base_config_test import ConfigTestBase


class ErrorPageMultipleCodesTest(ConfigTestBase):
    def get_name(self) -> str:
        return "error_page_multiple_codes"

    def get_description(self) -> str:
        return "Tests that error_page with multiple codes is accepted (nginx behavior)."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		error_page 404 500 502 503 /error.html;
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"error_page with multiple codes should be accepted: {result['stderr']}"
        )
        print("✓ Multiple error codes accepted")
