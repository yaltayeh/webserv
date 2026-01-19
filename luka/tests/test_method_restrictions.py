"""
HTTP Method restrictions test.
Tests that the server properly enforces allowed_methods directives.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class MethodRestrictionsTest(BaseTest):
    def get_name(self) -> str:
        return "method_restrictions"

    def get_description(self) -> str:
        return "Tests that server enforces HTTP method restrictions (405 Method Not Allowed)."

    def setup(self):
        # Create files in different locations
        self.create_html_file("index.html", HTMLBuilder.index_page("Method Test"))

        # GET-only location
        self.create_directory("readonly")
        self.create_html_file(
            "data.html",
            HTMLBuilder.simple_page("Read Only", "<h1>Read Only Content</h1>"),
            subdir="readonly",
        )

        # POST-only location (for form submissions)
        self.create_directory("submit")
        self.create_html_file(
            "form.html",
            HTMLBuilder.simple_page("Submit", "<h1>Submit Form</h1>"),
            subdir="submit",
        )

        # GET and POST allowed
        self.create_directory("standard")
        self.create_html_file(
            "page.html",
            HTMLBuilder.simple_page("Standard", "<h1>Standard Page</h1>"),
            subdir="standard",
        )

        # All methods allowed
        self.create_directory("api")
        self.create_file("resource.txt", "API resource", subdir="api")

        # Create configuration with method restrictions
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /readonly/ {{
            allowed_methods GET;
        }}
        
        location /submit/ {{
            allowed_methods POST;
        }}
        
        location /standard/ {{
            allowed_methods GET POST;
        }}
        
        location /api/ {{
            allowed_methods GET POST PUT DELETE;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: GET on GET-only location - should succeed (200)
        response = self.get("/readonly/data.html")
        self.assert_status(response, 200, "GET should be allowed on readonly location")

        # Test 2: POST on GET-only location - should return 405
        response_post = self.post("/readonly/data.html", data="test=data")
        self.assert_status(
            response_post, 405, "POST should not be allowed on GET-only location"
        )

        # Test 3: DELETE on GET-only location - should return 405
        response_delete = self.delete("/readonly/data.html")
        self.assert_status(
            response_delete, 405, "DELETE should not be allowed on GET-only location"
        )

        # Test 4: GET on POST-only location - should return 405
        response_get_post_only = self.get("/submit/form.html")
        self.assert_status(
            response_get_post_only,
            405,
            "GET should not be allowed on POST-only location",
        )

        # Test 5: POST on POST-only location - should succeed
        response_post_allowed = self.post("/submit/form.html", data="field=value")
        if response_post_allowed:
            # POST might return 200, 201, 204, or even 404 if file doesn't exist
            # but NOT 405
            assert response_post_allowed.status_code != 405, (
                f"POST should be allowed on POST-only location, got {response_post_allowed.status_code}"
            )

        # Test 6: GET and POST on standard location - both should work
        response_get_standard = self.get("/standard/page.html")
        self.assert_status(
            response_get_standard, 200, "GET should be allowed on standard location"
        )

        response_post_standard = self.post("/standard/page.html", data="data=test")
        if response_post_standard:
            assert response_post_standard.status_code != 405, (
                f"POST should be allowed on standard location, got {response_post_standard.status_code}"
            )

        # Test 7: All methods on /api/ location
        response_api_get = self.get("/api/resource.txt")
        self.assert_status(
            response_api_get, 200, "GET should be allowed on api location"
        )

        response_api_post = self.post("/api/resource.txt", data="data")
        if response_api_post:
            assert response_api_post.status_code != 405, (
                "POST should be allowed on api location"
            )

        response_api_put = self.put("/api/resource.txt", data="new data")
        if response_api_put:
            assert response_api_put.status_code != 405, (
                "PUT should be allowed on api location"
            )

        response_api_delete = self.delete("/api/resource.txt")
        if response_api_delete:
            assert response_api_delete.status_code != 405, (
                "DELETE should be allowed on api location"
            )

        # Test 8: DELETE on root location (only GET/POST allowed) - should return 405
        response_delete_root = self.delete("/index.html")
        self.assert_status(
            response_delete_root, 405, "DELETE should not be allowed on root location"
        )
