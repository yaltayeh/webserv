"""
HEAD vs GET comparison test.
Tests that HEAD method returns the same headers as GET but without the body.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class HeadVsGetTest(BaseTest):
    def get_name(self) -> str:
        return "head_vs_get"

    def get_description(self) -> str:
        return "Tests HEAD method returns same headers as GET but without body."

    def setup(self):
        # Create an HTML file
        html_content = HTMLBuilder.simple_page(
            title="Test Page",
            body="<h1>Testing HEAD vs GET</h1><p>This is the page content that should appear in GET but not HEAD.</p>",
        )
        self.create_html_file("index.html", html_content)

        # Create a larger file to make body size difference obvious
        large_content = HTMLBuilder.simple_page(
            title="Large Page",
            body="<h1>Large Content</h1>" + "<p>Lorem ipsum dolor sit amet.</p>" * 100,
        )
        self.create_html_file("large.html", large_content)

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET HEAD;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: HEAD and GET on index page
        head_response = self.head("/")
        get_response = self.get("/")

        # Both should succeed with 200
        self.assert_status(head_response, 200, "HEAD request should return 200")
        self.assert_status(get_response, 200, "GET request should return 200")

        # Check that both have Content-Type header
        self.assert_contains(
            str(head_response.headers.get("Content-Type", "")),
            "text/html",
            "HEAD response should have text/html Content-Type",
        )
        self.assert_contains(
            str(get_response.headers.get("Content-Type", "")),
            "text/html",
            "GET response should have text/html Content-Type",
        )

        # HEAD response should have no body or empty body
        head_body_len = len(head_response.text) if head_response.text else 0
        get_body_len = len(get_response.text) if get_response.text else 0

        self.assert_equal(
            True,
            head_body_len == 0 or head_body_len < 10,
            f"HEAD response should have no body (got {head_body_len} bytes)",
        )

        self.assert_equal(
            True,
            get_body_len > 50,
            f"GET response should have body content (got {get_body_len} bytes)",
        )

        # GET response should contain the actual content
        self.assert_contains(
            get_response.text,
            "Testing HEAD vs GET",
            "GET response should contain page content",
        )

        # Test 2: HEAD and GET on large page
        head_large = self.head("/large.html")
        get_large = self.get("/large.html")

        self.assert_status(head_large, 200, "HEAD to large.html should return 200")
        self.assert_status(get_large, 200, "GET to large.html should return 200")

        # Verify Content-Length headers exist (if server provides them)
        head_content_length = head_large.headers.get("Content-Length")
        get_content_length = get_large.headers.get("Content-Length")

        # If Content-Length is provided, it should match between HEAD and GET
        if head_content_length and get_content_length:
            self.assert_equal(
                head_content_length,
                get_content_length,
                "HEAD and GET should report same Content-Length",
            )

        # HEAD should have minimal/no body, GET should have large body
        head_large_body_len = len(head_large.text) if head_large.text else 0
        get_large_body_len = len(get_large.text) if get_large.text else 0

        self.assert_equal(
            True,
            head_large_body_len < 10,
            f"HEAD response for large file should have no body (got {head_large_body_len} bytes)",
        )

        self.assert_equal(
            True,
            get_large_body_len > 1000,
            f"GET response for large file should have substantial body (got {get_large_body_len} bytes)",
        )
