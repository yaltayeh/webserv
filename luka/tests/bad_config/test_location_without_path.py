"""
Location without path test.
Tests that location directive without path argument is rejected.
"""

from luka.base_config_test import ConfigTestBase


class LocationWithoutPathTest(ConfigTestBase):
    def get_name(self) -> str:
        return "location_without_path"

    def get_description(self) -> str:
        return "Tests that location without path is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location {{
			index index.html;
		}}
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], (
            "Location directive without path should be rejected"
        )
        print("✓ Location without path rejected")
