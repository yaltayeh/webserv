"""
Unclosed quote test.
Tests that configuration with unclosed quote is rejected.
"""

from luka.base_config_test import ConfigTestBase


class UnclosedQuoteTest(ConfigTestBase):
    def get_name(self) -> str:
        return "unclosed_quote"

    def get_description(self) -> str:
        return "Tests that config with unclosed quote is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name "localhost;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with unclosed quote should be rejected"
        print("✓ Unclosed quote rejected")
