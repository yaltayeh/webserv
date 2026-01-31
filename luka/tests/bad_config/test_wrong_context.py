"""
Wrong context test.
Tests that directive in wrong context is rejected.
"""

from luka.base_config_test import ConfigTestBase


class WrongContextTest(ConfigTestBase):
    def get_name(self) -> str:
        return "wrong_context"

    def get_description(self) -> str:
        return "Tests that directive in wrong context (listen in location) is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		
		location / {{
			listen 9090;
		}}
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with wrong context should be rejected"
        print("✓ Directive in wrong context rejected")
