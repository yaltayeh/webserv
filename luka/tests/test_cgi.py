"""
CGI test.
Tests if the server executes CGI scripts correctly.
"""
from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder, CGIBuilder


class CGIBasicTest(BaseTest):
    def get_name(self) -> str:
        return "cgi_basic"
    
    def get_description(self) -> str:
        return "Tests basic CGI script execution with Python."
    
    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("CGI Test"))
        
        # Create CGI scripts
        self.create_cgi_script("hello.py", CGIBuilder.python_hello_world())
        self.create_cgi_script("env.py", CGIBuilder.python_echo_env())
        self.create_cgi_script("query.py", CGIBuilder.python_query_string())
        
        # Create configuration with CGI support
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /cgi-bin/ {{
            allowed_methods GET POST;
            cgi_extension .py;
            cgi_pass {self.cgi_dir};
        }}
    }}
}}"""
        self.create_conf(conf)
    
    def run(self):
        # Test basic CGI script
        response = self.get("/cgi-bin/hello.py")
        self.assert_status(response, 200)
        self.assert_contains(response.text, "Hello from CGI!", "CGI script should output hello message")
        
        # Test CGI script with query string
        response_query = self.get("/cgi-bin/query.py?name=test&value=123")
        self.assert_status(response_query, 200)
        self.assert_contains(response_query.text, "name", "CGI script should process query parameters")
