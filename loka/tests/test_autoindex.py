"""
Autoindex test.
Tests if the server properly generates directory listings.
"""
from loka.base_test import BaseTest
from loka.utilities.test_helpers import HTMLBuilder


class AutoindexTest(BaseTest):
    def get_name(self) -> str:
        return "autoindex"
    
    def get_description(self) -> str:
        return "Tests if the server generates directory listings with autoindex."
    
    def setup(self):
        # Create multiple files in the www directory
        self.create_html_file("file1.html", HTMLBuilder.simple_page("File 1", "<h1>File 1</h1>"))
        self.create_html_file("file2.html", HTMLBuilder.simple_page("File 2", "<h1>File 2</h1>"))
        
        # Create a subdirectory with files
        self.create_directory("subdir")
        self.create_html_file("nested.html", HTMLBuilder.simple_page("Nested", "<h1>Nested File</h1>"), subdir="subdir")
        
        # Create configuration with autoindex enabled
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        autoindex on;
        
        location / {{
            allowed_methods GET;
            autoindex on;
        }}
    }}
}}"""
        self.create_conf(conf)
    
    def run(self):
        # Request the root directory (no index file, should show autoindex)
        response = self.get("/")
        
        self.assert_status(response, 200)
        
        # Check that the directory listing contains our files
        self.assert_contains(response.text, "file1.html", "Directory listing should contain file1.html")
        self.assert_contains(response.text, "file2.html", "Directory listing should contain file2.html")
        self.assert_contains(response.text, "subdir", "Directory listing should contain subdir")
        
        # Request subdirectory
        response_subdir = self.get("/subdir/")
        self.assert_status(response_subdir, 200)
        self.assert_contains(response_subdir.text, "nested.html", "Subdirectory listing should contain nested.html")
