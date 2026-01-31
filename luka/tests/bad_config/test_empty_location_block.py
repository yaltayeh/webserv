"""
Empty location block test.
Tests that empty location blocks are accepted.
"""

from luka.base_config_test import ConfigTestBase


class EmptyLocationBlockTest(ConfigTestBase):
    def get_name(self) -> str:
        return "empty_location_block"

    def get_description(self) -> str:
        return "Tests that empty location blocks are accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location / {{
		}}
		
		location /api {{
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Config with empty location blocks should be accepted: {result['stderr']}"
        )
        print("✓ Empty location blocks accepted")
