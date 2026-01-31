"""
error_page code 99 test.
Tests that error codes below 100 are rejected.
"""

from luka.base_config_test import ConfigTestBase


class ErrorPageCode99Test(ConfigTestBase):
    def get_name(self) -> str:
        return "error_page_code_99"

    def get_description(self) -> str:
        return "Tests that error_page with code < 100 is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		error_page 99 /error.html;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Error code 99 should be rejected (must be 100-599)"
        )
        print("✓ Error code 99 rejected")
