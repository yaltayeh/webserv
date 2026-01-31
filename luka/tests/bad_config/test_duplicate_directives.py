"""
Duplicate conflicting directives test.
Tests behavior with duplicate directives (may override like nginx).
"""

from luka.base_config_test import ConfigTestBase


class DuplicateDirectivesTest(ConfigTestBase):
    def get_name(self) -> str:
        return "duplicate_directives"

    def get_description(self) -> str:
        return "Tests behavior with duplicate directives (may be accepted or rejected)."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		server_name localhost;
		root {self.www_dir};
		client_max_body_size 1m;
		client_max_body_size 10m;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        # Duplicate directives may override (nginx-like) - accept either behavior
        if result["success"]:
            print("✓ Duplicate directive accepted (override behavior)")
        else:
            print("✓ Duplicate directive rejected")
