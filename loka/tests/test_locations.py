"""
Multiple locations test.
Tests if the server handles different location blocks correctly.
"""
from loka.base_test import BaseTest
from loka.utilities.test_helpers import HTMLBuilder


class MultipleLocationsTest(BaseTest):
    def get_name(self) -> str:
        return "multiple_locations"
    
    def get_description(self) -> str:
        return "Tests if the server routes requests to different location blocks correctly."
    
    def setup(self):
        # Create files in different directories
        self.create_html_file("index.html", HTMLBuilder.index_page("Root"))
        
        self.create_directory("api")
        self.create_html_file("data.html", 
                            HTMLBuilder.simple_page("API", "<h1>API Endpoint</h1>"),
                            subdir="api")
        
        self.create_directory("static")
        self.create_html_file("style.html",
                            HTMLBuilder.simple_page("Style", "<h1>Static Content</h1>"),
                            subdir="static")
        
        # Create configuration with multiple locations
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /api/ {{
            allowed_methods GET POST PUT DELETE;
            autoindex on;
        }}
        
        location /static/ {{
            allowed_methods GET;
            autoindex off;
        }}
    }}
}}"""
        self.create_conf(conf)
    
    def run(self):
        # Test root location
        response_root = self.get("/")
        self.assert_status(response_root, 200)
        self.assert_contains(response_root.text, "Root")
        
        # Test /api/ location
        response_api = self.get("/api/data.html")
        self.assert_status(response_api, 200)
        self.assert_contains(response_api.text, "API Endpoint")
        
        # Test /static/ location
        response_static = self.get("/static/style.html")
        self.assert_status(response_static, 200)
        self.assert_contains(response_static.text, "Static Content")
