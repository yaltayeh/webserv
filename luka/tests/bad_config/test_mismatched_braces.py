"""
Mismatched braces test.
Tests that configuration with mismatched braces is rejected.
"""

from luka.base_config_test import ConfigTestBase


class MismatchedBracesTest(ConfigTestBase):
    def get_name(self) -> str:
        return "mismatched_braces"

    def get_description(self) -> str:
        return "Tests that config with mismatched braces is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
	
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with mismatched braces should be rejected"
        print("✓ Mismatched braces rejected")
