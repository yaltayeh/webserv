"""
Unopened block test.
Tests that configuration with extra closing brace is rejected.
"""

from luka.base_config_test import ConfigTestBase


class UnopenedBlockTest(ConfigTestBase):
    def get_name(self) -> str:
        return "unopened_block"

    def get_description(self) -> str:
        return "Tests that config with extra closing brace is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
	}}
}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with extra closing brace should be rejected"
        )
        print("✓ Extra closing brace rejected")
