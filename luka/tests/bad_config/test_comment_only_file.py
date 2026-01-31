"""
Comment-only file test.
Tests that configuration with only comments is rejected.
"""

from luka.base_config_test import ConfigTestBase


class CommentOnlyFileTest(ConfigTestBase):
    def get_name(self) -> str:
        return "comment_only_file"

    def get_description(self) -> str:
        return "Tests that config with only comments is rejected (no server block)."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = """# This is a comment
# Another comment
# Yet another comment
# No actual configuration here
"""
        result = self.test_config(conf, webserv_binary)
        assert not result["success"], "Config with only comments should be rejected"
        print("✓ Comment-only file rejected")
