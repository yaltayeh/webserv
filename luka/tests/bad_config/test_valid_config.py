"""
Valid configuration test (baseline).
Tests that a properly formatted configuration is accepted.
"""

from luka.base_config_test import ConfigTestBase
from luka.utilities.test_helpers import HTMLBuilder


class ValidConfigTest(ConfigTestBase):
    def get_name(self) -> str:
        return "valid_config"

    def get_description(self) -> str:
        return "Tests that a valid configuration is accepted (baseline)."

    def setup(self):
        super().setup()
        # Create index page for valid config
        self.create_html_file("index.html", HTMLBuilder.index_page("Config Test"))

    def run(self):
        webserv_binary = self.get_webserv_binary()

        valid_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		index index.html;
		client_max_body_size 1m;
		
		location / {{
			allowed_methods GET POST;
		}}
	}}
}}"""
        result = self.test_config(valid_conf, webserv_binary)

        if not result["success"]:
            print(f"  Returncode: {result.get('returncode', 'N/A')}")
            print(f"  Stderr: {result.get('stderr', '')}")
            print(f"  Stdout: {result.get('stdout', '')}")

        assert result["success"], "Valid config should be accepted"
        print("✓ Valid configuration accepted")
