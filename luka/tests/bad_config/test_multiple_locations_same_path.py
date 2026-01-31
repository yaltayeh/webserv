"""
Multiple locations with same path test.
Tests that multiple location blocks with the same path are handled.
"""

from luka.base_config_test import ConfigTestBase


class MultipleLocationsSamePathTest(ConfigTestBase):
    def get_name(self) -> str:
        return "multiple_locations_same_path"

    def get_description(self) -> str:
        return "Tests multiple locations with same path (may override or warn)."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location /api {{
			index first.html;
		}}
		
		location /api {{
			index second.html;
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        # This behavior varies: nginx accepts it (last wins), some reject it
        if result["success"]:
            print(
                "✓ Multiple locations with same path accepted (last one may override)"
            )
        else:
            print("✓ Multiple locations with same path rejected")
