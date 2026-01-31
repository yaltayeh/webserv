"""
Invalid error code test.
Tests that configuration with invalid HTTP error code is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidErrorCodeTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_error_code"

    def get_description(self) -> str:
        return "Tests that config with invalid error code (1000) is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		error_page 1000 /error.html;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with invalid error code should be rejected"
        )
        print("✓ Invalid error code (1000) rejected")
