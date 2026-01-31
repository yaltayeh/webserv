"""
Deeply nested locations test.
Tests that deeply nested location blocks are handled correctly.
"""

from luka.base_config_test import ConfigTestBase


class DeeplyNestedLocationsTest(ConfigTestBase):
    def get_name(self) -> str:
        return "deeply_nested_locations"

    def get_description(self) -> str:
        return "Tests deeply nested location blocks."

    def run(self):
        webserv_binary = self.get_webserv_binary()

        # Note: Nested locations inside locations are typically not allowed in nginx
        # This test checks webserv's behavior
        conf = f"""http {{
	server {{
		listen {self.get_test_port()};
		root {self.www_dir};
		
		location / {{
			location /api {{
				location /v1 {{
				}}
			}}
		}}
	}}
}}"""
        result = self.test_config(conf, webserv_binary)
        # webserv may reject nested locations like nginx does
        # Document actual behavior
        if not result["success"]:
            print("✓ Nested locations rejected (expected nginx-like behavior)")
        else:
            print("✓ Nested locations accepted (differs from nginx)")
