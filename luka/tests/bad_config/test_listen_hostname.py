"""
Listen directive with hostname test.
Tests that listen with hostname:port format is accepted.
"""

from luka.base_config_test import ConfigTestBase


class ListenHostnameTest(ConfigTestBase):
    def get_name(self) -> str:
        return "listen_hostname"

    def get_description(self) -> str:
        return "Tests that listen with hostname:port is accepted."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        conf = f"""http {{
	server {{
		listen localhost:{self.get_test_port()};
		root {self.www_dir};
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        assert result["success"], (
            f"Listen with hostname should be accepted: {result['stderr']}"
        )
        print("✓ Hostname:port format accepted")
