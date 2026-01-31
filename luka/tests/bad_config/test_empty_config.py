"""
Empty configuration test.
Tests that empty configuration file is rejected.
"""

from luka.base_config_test import ConfigTestBase


class EmptyConfigTest(ConfigTestBase):
    def get_name(self) -> str:
        return "empty_config"

    def get_description(self) -> str:
        return "Tests that empty configuration is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = ""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Empty config should be rejected"
        print("✓ Empty configuration rejected")
