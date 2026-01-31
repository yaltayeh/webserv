"""
Mixed indentation test.
Tests that configuration with mixed tabs and spaces is accepted.
"""

from luka.base_config_test import ConfigTestBase


class MixedIndentationTest(ConfigTestBase):
    def get_name(self) -> str:
        return "mixed_indentation"

    def get_description(self) -> str:
        return "Tests that config with mixed tabs and spaces is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        # Mix of tabs and spaces for indentation
        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
  server_name localhost;
    root {self.www_dir};
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with mixed indentation should be accepted: {result['stderr']}"
        )
        print("✓ Mixed indentation accepted")
