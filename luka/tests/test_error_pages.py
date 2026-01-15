"""
Error pages test.
Tests if the server serves custom error pages.
"""
from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class ErrorPagesTest(BaseTest):
    def get_name(self) -> str:
        return "error_pages"
    
    def get_description(self) -> str:
        return "Tests if the server serves custom error pages for 404 and other errors."
    
    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("Home"))
        
        # Create custom error pages
        self.create_html_file("404.html", HTMLBuilder.error_page(404, "Page not found"))
        self.create_html_file("403.html", HTMLBuilder.error_page(403, "Forbidden"))
        
        # Create configuration with error pages
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        error_page 404 /404.html;
        error_page 403 /403.html;
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""
        self.create_conf(conf)
    
    def run(self):
        # Test that index page works
        response = self.get("/")
        self.assert_status(response, 200)
        self.assert_contains(response.text, "Home")
        
        # Test 404 error page
        response_404 = self.get("/nonexistent.html")
        self.assert_status(response_404, 404)
        self.assert_contains(response_404.text, "Page not found", "404 page should contain custom error message")
