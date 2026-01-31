"""
allowed_methods empty test.
Tests that allowed_methods without arguments is rejected.
"""

from luka.base_config_test import ConfigTestBase


class AllowedMethodsEmptyTest(ConfigTestBase):
    def get_name(self) -> str:
        return "allowed_methods_empty"

    def get_description(self) -> str:
        return "Tests that allowed_methods without arguments is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location / {{
			allowed_methods;
		}}
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "allowed_methods without arguments should be rejected"
        )
        print("✓ Empty allowed_methods rejected")
