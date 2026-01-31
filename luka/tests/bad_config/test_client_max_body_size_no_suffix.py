"""
client_max_body_size no suffix test.
Tests that size values without suffix (bytes) are accepted.
"""

from luka.base_config_test import ConfigTestBase


class ClientMaxBodySizeNoSuffixTest(ConfigTestBase):
    def get_name(self) -> str:
        return "client_max_body_size_no_suffix"

    def get_description(self) -> str:
        return "Tests that client_max_body_size without suffix (bytes) is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		client_max_body_size 1024;
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Size without suffix should be accepted: {result['stderr']}"
        )
        print("✓ Size in bytes (no suffix) accepted")
