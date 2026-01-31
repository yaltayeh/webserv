"""
return no args test.
Tests that return directive without arguments is rejected.
"""

from luka.base_config_test import ConfigTestBase


class ReturnNoArgsTest(ConfigTestBase):
    def get_name(self) -> str:
        return "return_no_args"

    def get_description(self) -> str:
        return "Tests that return without arguments is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location / {{
			return;
		}}
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "return without arguments should be rejected"
        print("✓ return without args rejected")
