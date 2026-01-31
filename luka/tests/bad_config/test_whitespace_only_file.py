"""
Whitespace-only file test.
Tests that configuration with only whitespace is rejected.
"""

from luka.base_config_test import ConfigTestBase


class WhitespaceOnlyFileTest(ConfigTestBase):
    def get_name(self) -> str:
        return "whitespace_only_file"

    def get_description(self) -> str:
        return "Tests that config with only whitespace is rejected (empty config)."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = """


	
		
			

"""
        result = self.test_config(conf, webserv_binary)
        assert not result["success"], "Config with only whitespace should be rejected"
        print("✓ Whitespace-only file rejected")
