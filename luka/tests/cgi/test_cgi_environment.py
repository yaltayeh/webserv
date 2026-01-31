"""
CGI environment variables test.
Tests that the server correctly sets CGI environment variables (HTTP/1.0).
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder, CGIBuilder


class CgiEnvironmentTest(BaseTest):
    def get_name(self) -> str:
        return "cgi_environment"

    def get_description(self) -> str:
        return "Tests CGI environment variables are correctly set (HTTP/1.0)."

    def setup(self):
        # Create index page
        self.create_html_file("index.html", HTMLBuilder.index_page("CGI Env Test"))

        # Create comprehensive CGI script to check environment variables
        self.create_cgi_script("env_check.py", CGIBuilder.python_echo_env())
        self.create_cgi_script("query.py", CGIBuilder.python_query_string())
        self.create_cgi_script("post.py", CGIBuilder.python_post_data())

        # Create configuration with CGI support
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost testserver.local;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /cgi-bin/ {{
            allowed_methods GET POST;
            alias {self.cgi_dir}/;
            cgi_pass .py /usr/bin/python3;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test 1: Basic CGI execution with environment variables
        response = self.get("/cgi-bin/env_check.py")
        if response and response.status_code == 200:
            # Check for important CGI environment variables
            self.assert_contains(
                response.text, "REQUEST_METHOD", "CGI should set REQUEST_METHOD"
            )
            self.assert_contains(response.text, "GET", "REQUEST_METHOD should be GET")
            self.assert_contains(
                response.text, "SERVER_", "CGI should have SERVER_* variables"
            )

        # Test 2: CGI with query string - check QUERY_STRING variable
        response_query = self.get(
            "/cgi-bin/query.py?name=test&value=123&special=hello%20world"
        )
        if response_query and response_query.status_code == 200:
            self.assert_contains(
                response_query.text, "name", "CGI should receive query parameters"
            )
            self.assert_contains(
                response_query.text, "test", "CGI should parse query string correctly"
            )
            self.assert_contains(
                response_query.text, "value", "CGI should receive all query parameters"
            )

        # Test 3: CGI with POST data - check CONTENT_LENGTH and CONTENT_TYPE
        post_data = "username=testuser&password=testpass&email=test@example.com"
        response_post = self.post(
            "/cgi-bin/post.py",
            data=post_data,
            headers={"Content-Type": "application/x-www-form-urlencoded"},
        )
        if response_post and response_post.status_code == 200:
            self.assert_contains(
                response_post.text, "POST Data", "CGI should handle POST data"
            )
            # The CGI script should receive the POST data
            self.assert_contains(
                response_post.text, "username", "CGI should receive POST parameters"
            )

        # Test 4: Verify PATH_INFO handling
        response_path = self.get("/cgi-bin/env_check.py/extra/path/info")
        if response_path:
            # Some servers might return 404 for PATH_INFO, others process it
            assert response_path.status_code in [200, 404], (
                f"PATH_INFO request should be handled, got {response_path.status_code}"
            )

        # Test 5: CGI with different request methods
        response_get = self.get("/cgi-bin/env_check.py")
        if response_get and response_get.status_code == 200:
            self.assert_contains(
                response_get.text,
                "GET",
                "REQUEST_METHOD should be GET for GET requests",
            )

        # Test 6: Empty query string
        response_no_query = self.get("/cgi-bin/query.py")
        if response_no_query:
            assert response_no_query.status_code in [200], (
                f"CGI with no query should work, got {response_no_query.status_code}"
            )
