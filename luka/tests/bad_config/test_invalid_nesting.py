"""
Invalid nesting test.
Tests that invalid block nesting is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidNestingTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_nesting"

    def get_description(self) -> str:
        return "Tests that invalid nesting (server in location) is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		
		location / {{
			server {{
				listen 9090;
			}}
		}}
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with invalid nesting should be rejected"
        print("✓ Invalid nesting rejected")
