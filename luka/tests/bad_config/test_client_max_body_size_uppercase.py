"""
client_max_body_size uppercase suffix test.
Tests that uppercase suffixes (K/M/G) are accepted.
"""

from luka.base_config_test import ConfigTestBase


class ClientMaxBodySizeUppercaseTest(ConfigTestBase):
    def get_name(self) -> str:
        return "client_max_body_size_uppercase"

    def get_description(self) -> str:
        return "Tests that client_max_body_size with uppercase suffixes is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		client_max_body_size 10M;
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Uppercase suffix 'M' should be accepted: {result['stderr']}"
        )
        print("✓ Uppercase size suffix accepted")
