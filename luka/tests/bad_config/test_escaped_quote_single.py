"""
Escaped single quote test.
Tests that escaped single quotes in strings are accepted.
"""

from luka.base_config_test import ConfigTestBase


class EscapedQuoteSingleTest(ConfigTestBase):
    def get_name(self) -> str:
        return "escaped_quote_single"

    def get_description(self) -> str:
        return "Tests that escaped single quotes are accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		
		location / {{
			return 200 'It\\'s working';
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with escaped single quotes should be accepted: {result['stderr']}"
        )
        print("✓ Escaped single quotes accepted")
