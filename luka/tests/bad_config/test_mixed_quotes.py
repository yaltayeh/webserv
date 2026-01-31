"""
Mixed quotes test.
Tests that configuration with both single and double quotes is accepted.
"""

from luka.base_config_test import ConfigTestBase


class MixedQuotesTest(ConfigTestBase):
    def get_name(self) -> str:
        return "mixed_quotes"

    def get_description(self) -> str:
        return "Tests that config with mixed quote types is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name "localhost";
		root '{self.www_dir}';
		
		location / {{
			index 'index.html' "default.html";
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with mixed quote types should be accepted: {result['stderr']}"
        )
        print("✓ Mixed quotes accepted")
