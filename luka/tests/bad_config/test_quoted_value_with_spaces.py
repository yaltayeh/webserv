"""
Quoted value with spaces test.
Tests that quoted values containing spaces are accepted.
"""

from luka.base_config_test import ConfigTestBase


class QuotedValueWithSpacesTest(ConfigTestBase):
    def get_name(self) -> str:
        return "quoted_value_with_spaces"

    def get_description(self) -> str:
        return "Tests that quoted values with spaces are accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name "localhost with spaces";
		root "{self.www_dir}/path with spaces";
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with quoted values containing spaces should be accepted: {result['stderr']}"
        )
        print("✓ Quoted values with spaces accepted")
