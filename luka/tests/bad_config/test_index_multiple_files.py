"""
index multiple files test.
Tests that index with multiple files is accepted.
"""

from luka.base_config_test import ConfigTestBase


class IndexMultipleFilesTest(ConfigTestBase):
    def get_name(self) -> str:
        return "index_multiple_files"

    def get_description(self) -> str:
        return "Tests that index with multiple files is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		index index.html index.htm default.html home.html;
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"index with multiple files should be accepted: {result['stderr']}"
        )
        print("✓ index with multiple files accepted")
