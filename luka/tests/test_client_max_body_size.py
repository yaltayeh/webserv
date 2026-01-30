"""
Test client_max_body_size directive.
Tests various request body size limits and edge cases with 413 Payload Too Large responses.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class ClientMaxBodySizeTest(BaseTest):
    def get_name(self) -> str:
        return "client_max_body_size"

    def get_description(self) -> str:
        return "Tests client_max_body_size directive with various body sizes and HTTP methods."

    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("Body Size Test"))

        # Create custom 413 error page
        self.create_html_file(
            "413.html",
            HTMLBuilder.simple_page(
                "413 Payload Too Large",
                "<h1>413 Payload Too Large</h1><p>Your request body is too large.</p>",
            ),
        )

        # Create configuration with body size limit at server level
        # Note: client_max_body_size can only be in http or server context (not location)
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 1k;
        error_page 413 /413.html;
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: GET request to verify server is running
        response = self.get("/")
        self.assert_status(response, 200, "Server should respond to GET")

        # Test 2: POST with body under limit (1k = 1024 bytes)
        # Send 512 bytes - should succeed
        small_body = "x" * 512
        response = self.post(
            "/", data=small_body, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(
            response, 200, "POST with 512 bytes should succeed (limit is 1k)"
        )

        # Test 3: POST with body exactly at limit (1024 bytes)
        # Should succeed - exactly at the limit
        exact_body = "x" * 1024
        response = self.post(
            "/", data=exact_body, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(response, 200, "POST with exactly 1024 bytes should succeed")

        # Test 4: POST with body 1 byte under limit (1023 bytes)
        # Should succeed
        under_limit = "x" * 1023
        response = self.post(
            "/", data=under_limit, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(response, 200, "POST with 1023 bytes should succeed")

        # Test 5: POST with body 1 byte over limit (1025 bytes)
        # Should return 413 Payload Too Large
        over_limit = "x" * 1025
        response = self.post(
            "/", data=over_limit, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(response, 413, "POST with 1025 bytes should return 413")
        self.assert_contains(
            response.text, "413", "Response should contain 413 error page"
        )

        # Test 6: POST with body significantly over limit (10k = 10240 bytes)
        # Should return 413
        large_body = "x" * 10240
        response = self.post(
            "/", data=large_body, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(
            response, 413, "POST with 10k should return 413 (limit is 1k)"
        )

        # Test 7: Empty body (0 bytes) - should always succeed
        response = self.post("/", data="", headers={"Content-Type": "text/plain"})
        self.assert_status(response, 200, "POST with empty body should succeed")

        # Test 8: Very small body (1 byte) - should succeed
        tiny_body = "x"
        response = self.post(
            "/", data=tiny_body, headers={"Content-Type": "text/plain"}
        )
        self.assert_status(response, 200, "POST with 1 byte should succeed")

        print("✓ All client_max_body_size tests passed!")
