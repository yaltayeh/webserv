"""
client_max_body_size negative test.
Tests that negative values are rejected.
"""

from luka.base_config_test import ConfigTestBase


class ClientMaxBodySizeNegativeTest(ConfigTestBase):
    def get_name(self) -> str:
        return "client_max_body_size_negative"

    def get_description(self) -> str:
        return "Tests that client_max_body_size with negative value is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		client_max_body_size -1;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Negative client_max_body_size should be rejected"
        print("✓ Negative client_max_body_size rejected")
