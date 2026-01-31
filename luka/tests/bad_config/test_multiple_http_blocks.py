"""
Multiple http blocks test.
Tests that configuration with multiple http blocks is rejected.
"""

from luka.base_config_test import ConfigTestBase


class MultipleHttpBlocksTest(ConfigTestBase):
    def get_name(self) -> str:
        return "multiple_http_blocks"

    def get_description(self) -> str:
        return "Tests that config with multiple http blocks is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
	}}
}}
http {{
	server {{
		listen 9090;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Config with multiple http blocks should be rejected"
        )
        print("✓ Multiple http blocks rejected")
