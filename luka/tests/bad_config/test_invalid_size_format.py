"""
Invalid size format test.
Tests that configuration with invalid size format is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidSizeFormatTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_size_format"

    def get_description(self) -> str:
        return "Tests that config with invalid size format is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		client_max_body_size abc;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with invalid size format should be rejected"
        )
        print("✓ Invalid size format ('abc') rejected")
