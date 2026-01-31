"""
Invalid boolean value test.
Tests that configuration with invalid boolean value is rejected.
"""

from luka.base_config_test import ConfigTestBase


class InvalidBooleanTest(ConfigTestBase):
    def get_name(self) -> str:
        return "invalid_boolean"

    def get_description(self) -> str:
        return "Tests that config with invalid boolean value is rejected."

    def setup(self):
        super().setup()
        # Create uploads directory
        import os

        os.makedirs(self.uploads_dir, exist_ok=True)

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		
		location /upload {{
			upload_enable maybe;
			upload_store {self.uploads_dir};
		}}
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Config with invalid boolean should be rejected"
        print("✓ Invalid boolean value ('maybe') rejected")
