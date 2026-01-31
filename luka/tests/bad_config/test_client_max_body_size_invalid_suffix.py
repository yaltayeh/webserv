"""
client_max_body_size invalid suffix test.
Tests that invalid size suffixes are rejected.
"""

from luka.base_config_test import ConfigTestBase


class ClientMaxBodySizeInvalidSuffixTest(ConfigTestBase):
    def get_name(self) -> str:
        return "client_max_body_size_invalid_suffix"

    def get_description(self) -> str:
        return "Tests that client_max_body_size with invalid suffix is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		client_max_body_size 10x;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Invalid suffix 'x' should be rejected (valid: k, m, g)"
        )
        print("✓ Invalid size suffix rejected")
