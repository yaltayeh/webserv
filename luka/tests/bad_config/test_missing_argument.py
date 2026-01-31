"""
Missing required argument test.
Tests that directive without required argument is rejected.
"""

from luka.base_config_test import ConfigTestBase


class MissingArgumentTest(ConfigTestBase):
    def get_name(self) -> str:
        return "missing_argument"

    def get_description(self) -> str:
        return "Tests that directive with missing argument is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		client_max_body_size;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with missing argument should be rejected"
        print("✓ Missing required argument rejected")
