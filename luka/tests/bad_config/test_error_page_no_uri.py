"""
error_page no URI test.
Tests that error_page without URI argument is rejected.
"""

from luka.base_config_test import ConfigTestBase


class ErrorPageNoURITest(ConfigTestBase):
    def get_name(self) -> str:
        return "error_page_no_uri"

    def get_description(self) -> str:
        return "Tests that error_page without URI is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		error_page 404;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "error_page without URI should be rejected"
        print("✓ error_page without URI rejected")
