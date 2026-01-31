"""
No server block test.
Tests that http block without server block is rejected.
"""

from luka.base_config_test import ConfigTestBase


class NoServerBlockTest(ConfigTestBase):
    def get_name(self) -> str:
        return "no_server_block"

    def get_description(self) -> str:
        return "Tests that config without server block is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = """http {
	worker_connections 1024;
}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config without server block should be rejected"
        print("✓ Config without server block rejected")
