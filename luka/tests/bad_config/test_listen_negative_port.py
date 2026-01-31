"""
Listen directive negative port test.
Tests that negative port numbers are rejected.
"""

from luka.base_config_test import ConfigTestBase


class ListenNegativePortTest(ConfigTestBase):
    def get_name(self) -> str:
        return "listen_negative_port"

    def get_description(self) -> str:
        return "Tests that listen directive with negative port is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen -1;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Listen with negative port should be rejected"
        print("✓ Negative port rejected")
