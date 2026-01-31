"""
return URL only test.
Tests that return with only URL (implicit 302) is accepted.
"""

from luka.base_config_test import ConfigTestBase


class ReturnURLOnlyTest(ConfigTestBase):
    def get_name(self) -> str:
        return "return_url_only"

    def get_description(self) -> str:
        return "Tests that return with URL only (implicit 302) is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location / {{
			return https://example.com;
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"return with URL only should be accepted (implicit 302): {result['stderr']}"
        )
        print("✓ return with URL only accepted")
