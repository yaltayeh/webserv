"""
error_page code 600 test.
Tests that error codes above 599 are rejected.
"""

from luka.base_config_test import ConfigTestBase


class ErrorPageCode600Test(ConfigTestBase):
    def get_name(self) -> str:
        return "error_page_code_600"

    def get_description(self) -> str:
        return "Tests that error_page with code > 599 is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		error_page 600 /error.html;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Error code 600 should be rejected (must be 100-599)"
        )
        print("✓ Error code 600 rejected")
