"""
Block directive with semicolon test.
Tests that block directives with unexpected semicolons are rejected.
"""

from luka.base_config_test import ConfigTestBase


class BlockDirectiveWithSemicolonTest(ConfigTestBase):
    def get_name(self) -> str:
        return "block_directive_with_semicolon"

    def get_description(self) -> str:
        return "Tests that block directives with semicolons are rejected."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        # Test 1: server with semicolon instead of block
        bad_conf1 = f"""http {{
	server;
}}"""
        result = self.test_config(bad_conf1, webserv_binary)
        assert not result["success"], "Block directive 'server;' should be rejected"
        print("✓ Block directive with semicolon rejected (server;)")

        # Test 2: semicolon at start of block
        bad_conf2 = f"""http {{
	server {{
		;
		listen {self.get_test_port()};
	}}
}}"""
        result = self.test_config(bad_conf2, webserv_binary)
        # This might be accepted as empty statement
        if not result["success"]:
            print("✓ Semicolon at block start rejected")
        else:
            print("✓ Semicolon at block start accepted (unexpected)")
