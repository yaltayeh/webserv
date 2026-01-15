"""
Basic GET request test.
Tests if the server responds correctly to a simple GET request.
"""
from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder, ConfigBuilder


class BasicRequestTest(BaseTest):
    def get_name(self) -> str:
        return "basic_request"
    
    def get_description(self) -> str:
        return "Tests if the server responds to a basic GET request."
    
    def setup(self):
        # Create the HTML file
        html_content = HTMLBuilder.simple_page(
            title="Welcome",
            body="<h1>Hello from webserv!</h1><p>This is the index page.</p>"
        )
        self.create_html_file("index.html", html_content)
        
        # Create configuration
        conf = ConfigBuilder.basic_server(
            port=8080,
            root=self.www_dir,
            server_name="localhost"
        )
        self.create_conf(conf)
    
    def run(self):
        response = self.get("/")
        
        # Assert status code is 200
        self.assert_status(response, 200)
        
        # Assert response contains expected content
        self.assert_contains(response.text, "Hello from webserv!")
        self.assert_contains(response.text, "This is the index page.")

