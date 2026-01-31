"""
Listen directive with IPv4 address test.
Tests that listen with IP:port format is accepted.
"""

from luka.base_config_test import ConfigTestBase


class ListenIPv4Test(ConfigTestBase):
    def get_name(self) -> str:
        return "listen_ipv4"

    def get_description(self) -> str:
        return "Tests that listen with IPv4:port is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen 127.0.0.1:{self.get_test_port()};
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Listen with IPv4 should be accepted: {result['stderr']}"
        )
        print("✓ IPv4:port format accepted")
