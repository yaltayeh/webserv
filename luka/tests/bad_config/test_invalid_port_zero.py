"""
Invalid port number test (zero).
Tests that configuration with port 0 is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidPortZeroTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_port_zero"

    def get_description(self) -> str:
        return "Tests that config with port 0 is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen 0;
		server_name localhost;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with port 0 should be rejected"
        print("✓ Port 0 rejected")
