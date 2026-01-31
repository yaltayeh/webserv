"""
Unclosed server block test.
Tests that configuration with unclosed server block is rejected.
"""

from luka.base_config_test import ConfigTestBase


class UnclosedServerBlockTest(ConfigTestBase):
    def get_name(self) -> str:
        return "unclosed_server_block"

    def get_description(self) -> str:
        return "Tests that config with unclosed server block is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
	
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with unclosed server block should be rejected"
        )
        print("✓ Unclosed server block rejected")
