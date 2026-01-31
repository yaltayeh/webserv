"""
Missing semicolon test.
Tests that configuration missing a semicolon is rejected.
"""

from luka.base_config_test import ConfigTestBase


class MissingSemicolonTest(ConfigTestBase):
    def get_name(self) -> str:
        return "missing_semicolon"

    def get_description(self) -> str:
        return "Tests that config with missing semicolon is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()}
		server_name localhost;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with missing semicolon should be rejected"
        print("✓ Missing semicolon rejected")
