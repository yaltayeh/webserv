"""
autoindex_format invalid test.
Tests that autoindex_format with invalid format is rejected.
"""

from luka.base_config_test import ConfigTestBase


class AutoindexFormatInvalidTest(ConfigTestBase):
    def get_name(self) -> str:
        return "autoindex_format_invalid"

    def get_description(self) -> str:
        return "Tests that autoindex_format with invalid format is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		autoindex_format text;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "autoindex_format 'text' should be rejected (valid: html, xml, json, jsonp)"
        )
        print("✓ Invalid autoindex_format rejected")
