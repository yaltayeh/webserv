"""
Invalid directive characters test.
Tests that directive with invalid characters is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidDirectiveCharactersTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_directive_characters"

    def get_description(self) -> str:
        return "Tests that directive with invalid characters is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		client@max!body#size 1m;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with invalid chars in directive should be rejected"
        )
        print("✓ Invalid directive characters rejected")
