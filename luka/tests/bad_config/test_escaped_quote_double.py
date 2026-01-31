"""
Escaped double quote test.
Tests that escaped double quotes in strings are accepted.
"""

from luka.base_config_test import ConfigTestBase


class EscapedQuoteDoubleTest(ConfigTestBase):
    def get_name(self) -> str:
        return "escaped_quote_double"

    def get_description(self) -> str:
        return "Tests that escaped double quotes are accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		
		location / {{
			return 200 "He said \\"hello\\"";
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with escaped double quotes should be accepted: {result['stderr']}"
        )
        print("✓ Escaped double quotes accepted")
