"""
Invalid port number test (too high).
Tests that configuration with port > 65535 is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidPortTooHighTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_port_too_high"

    def get_description(self) -> str:
        return "Tests that config with port 99999 is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen 99999;
		server_name localhost;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with invalid port should be rejected"
        print("✓ Invalid port (99999) rejected")
