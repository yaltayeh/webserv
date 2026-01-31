"""
index no args test.
Tests that index directive without arguments is rejected.
"""

from luka.base_config_test import ConfigTestBase


class IndexNoArgsTest(ConfigTestBase):
    def get_name(self) -> str:
        return "index_no_args"

    def get_description(self) -> str:
        return "Tests that index without arguments is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		index;
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "index without arguments should be rejected"
        print("✓ index without args rejected")
