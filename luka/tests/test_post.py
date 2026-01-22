"""
POST request test.
Tests if the server handles POST requests correctly.
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder, CGIBuilder


class PostRequestTest(BaseTest):
    def get_name(self) -> str:
        return "post_request"

    def get_description(self) -> str:
        return "Tests if the server handles POST requests with form data."

    def setup(self):
        # Create a form page
        self.create_html_file(
            "form.html", HTMLBuilder.form_page(action="/cgi-bin/post.py", method="POST")
        )

        # Create CGI script to handle POST data
        self.create_cgi_script("post.py", CGIBuilder.python_post_data())

        # Create configuration
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
        # Test GET request to form page
        response = self.get("/form.html")
        self.assert_status(response, 200)
        self.assert_contains(response.text, "form", "Form page should contain a form")

        # Test POST request with form data
        post_data = "username=testuser&password=testpass"
        response_post = self.post(
            "/cgi-bin/post.py",
            data=post_data,
            headers={"Content-Type": "application/x-www-form-urlencoded"},
        )
        self.assert_status(response_post, 200)
        self.assert_contains(
            response_post.text, "POST Data", "Response should acknowledge POST data"
        )
