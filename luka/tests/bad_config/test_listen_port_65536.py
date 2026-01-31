"""
Listen directive port 65536 test.
Tests that port numbers above 65535 are rejected.
"""

from luka.base_config_test import ConfigTestBase


class ListenPort65536Test(ConfigTestBase):
    def get_name(self) -> str:
        return "listen_port_65536"

    def get_description(self) -> str:
        return "Tests that listen directive with port > 65535 is rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        bad_conf = f"""http {{
	server {{
		listen 65536;
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(bad_conf, webserv_binary)
        assert not result["success"], "Listen with port 65536 should be rejected"
        print("✓ Port 65536 rejected")
