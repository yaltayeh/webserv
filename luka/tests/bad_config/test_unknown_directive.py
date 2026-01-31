"""
Unknown directive test.
Tests that configuration with unknown directive is rejected.
"""

from luka.base_config_test import ConfigTestBase


class UnknownDirectiveTest(ConfigTestBase):
    def get_name(self) -> str:
        return "unknown_directive"

    def get_description(self) -> str:
        return "Tests that config with unknown directive is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		unknown_directive value;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with unknown directive should be rejected"
        print("✓ Unknown directive rejected")
