"""
Empty server block test.
Tests that empty server blocks are accepted.
"""

from luka.base_config_test import ConfigTestBase


class EmptyServerBlockTest(ConfigTestBase):
    def get_name(self) -> str:
        return "empty_server_block"

    def get_description(self) -> str:
        return "Tests that empty server blocks are accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
	}}
	server {{
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with empty server block should be accepted: {result['stderr']}"
        )
        print("✓ Empty server block accepted")
