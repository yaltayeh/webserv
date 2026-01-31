"""
autoindex invalid value test.
Tests that autoindex with invalid boolean values is rejected.
"""

from luka.base_config_test import ConfigTestBase


class AutoindexInvalidValueTest(ConfigTestBase):
    def get_name(self) -> str:
        return "autoindex_invalid_value"

    def get_description(self) -> str:
        return "Tests that autoindex with invalid value (not on/off) is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		autoindex yes;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "autoindex 'yes' should be rejected (must be 'on' or 'off')"
        )
        print("✓ Invalid autoindex value rejected")
