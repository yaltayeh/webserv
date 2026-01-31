"""
Inline comment test.
Tests that configuration with inline comments is accepted.
"""

from luka.base_config_test import ConfigTestBase


class CommentInlineTest(ConfigTestBase):
    def get_name(self) -> str:
        return "comment_inline"

    def get_description(self) -> str:
        return "Tests that config with inline comments is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{  # HTTP block
	server {{  # Server block
		listen {self.get_test_port()};  # Listen directive
		server_name localhost;  # Server name
		root {self.www_dir};  # Root directory
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with inline comments should be accepted: {result['stderr']}"
        )
        print("✓ Inline comments accepted")
