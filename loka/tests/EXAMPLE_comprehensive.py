"""
Comprehensive example test demonstrating all framework features.
This is a template you can copy and modify for your own tests.
"""
from loka.base_test import BaseTest
from loka.utilities.test_helpers import HTMLBuilder, CGIBuilder, ConfigBuilder


class ComprehensiveExampleTest(BaseTest):
    """
    A comprehensive example showing all features of the testing framework.
    """
    
    def get_name(self) -> str:
        """Return unique test identifier."""
        return "comprehensive_example"
    
    def get_description(self) -> str:
        """Return human-readable test description."""
        return "Comprehensive example demonstrating all framework features."
    
    def setup(self):
        """
        Setup phase: Create all test resources.
        This runs before the server starts.
        """
        
        # ============================================================
        # 1. CREATE HTML FILES
        # ============================================================
        
        # Simple index page
        self.create_html_file(
            "index.html",
            HTMLBuilder.index_page("Test Server")
        )
        
        # About page
        self.create_html_file(
            "about.html",
            HTMLBuilder.simple_page(
                title="About",
                body="<h1>About Us</h1><p>This is the about page.</p>"
            )
        )
        
        # Custom error pages
        self.create_html_file("404.html", HTMLBuilder.error_page(404, "Page Not Found"))
        self.create_html_file("500.html", HTMLBuilder.error_page(500, "Internal Server Error"))
        
        # ============================================================
        # 2. CREATE DIRECTORY STRUCTURE
        # ============================================================
        
        # Create subdirectories
        self.create_directory("images", parent="www")
        self.create_directory("css", parent="www")
        self.create_directory("js", parent="www")
        
        # Create files in subdirectories
        self.create_file(
            "style.css",
            "body { font-family: Arial; color: #333; }",
            subdir="css"
        )
        
        self.create_file(
            "script.js",
            "console.log('Hello from JavaScript');",
            subdir="js"
        )
        
        self.create_html_file(
            "nested.html",
            HTMLBuilder.simple_page("Nested", "<h1>Nested Page</h1>"),
            subdir="images"
        )
        
        # ============================================================
        # 3. CREATE CGI SCRIPTS (if testing CGI)
        # ============================================================
        
        # Python CGI scripts
        self.create_cgi_script("hello.py", CGIBuilder.python_hello_world())
        self.create_cgi_script("env.py", CGIBuilder.python_echo_env())
        self.create_cgi_script("query.py", CGIBuilder.python_query_string())
        self.create_cgi_script("post.py", CGIBuilder.python_post_data())
        
        # Bash CGI script
        self.create_cgi_script("hello.sh", CGIBuilder.bash_hello_world())
        
        # ============================================================
        # 4. CREATE CONFIGURATION
        # ============================================================
        
        # Option A: Use ConfigBuilder for simple configs
        # conf = ConfigBuilder.basic_server(port=8080, root=self.www_dir)
        
        # Option B: Write custom configuration with automatic path substitution
        # Available placeholders: {www_root}, {uploads_dir}, {cgi_dir}, {logs_dir}, {port}
        conf = f"""http {{
    server {{
        listen {{port}};
        server_name localhost testserver.local;
        root {{www_root}};
        index index.html;
        autoindex on;
        client_max_body_size 10M;
        
        # Custom error pages
        error_page 404 /404.html;
        error_page 500 /500.html;
        
        # Root location
        location / {{
            allowed_methods GET POST;
            autoindex on;
        }}
        
        # Static files location
        location /static/ {{
            allowed_methods GET;
            autoindex off;
        }}
        
        # CGI location
        location /cgi-bin/ {{
            allowed_methods GET POST;
            cgi_extension .py .sh;
            cgi_pass {{cgi_dir}};
        }}
        
        # Upload location
        location /upload {{
            allowed_methods POST PUT DELETE;
            upload_enable on;
            upload_store {{uploads_dir}};
        }}
    }}
}}"""
        
        # Create the configuration file (placeholders are automatically replaced)
        self.create_conf(conf)
        
        # Alternatively, provide custom replacements:
        # self.create_conf(conf, replacements={"{custom}": "value"})
    
    def run(self):
        """
        Test execution phase: Make HTTP requests and verify responses.
        This runs after the server has started.
        """
        
        # ============================================================
        # TEST BASIC GET REQUESTS
        # ============================================================
        
        # Test index page
        response = self.get("/")
        self.assert_status(response, 200, "Index page should return 200")
        self.assert_contains(response.text, "Test Server", "Index should contain title")
        
        # Test about page
        response_about = self.get("/about.html")
        self.assert_status(response_about, 200)
        self.assert_contains(response_about.text, "About Us")
        
        # ============================================================
        # TEST STATIC FILES
        # ============================================================
        
        # Test CSS file
        response_css = self.get("/css/style.css")
        self.assert_status(response_css, 200)
        self.assert_contains(response_css.text, "font-family")
        
        # Test JS file
        response_js = self.get("/js/script.js")
        self.assert_status(response_js, 200)
        self.assert_contains(response_js.text, "console.log")
        
        # ============================================================
        # TEST ERROR PAGES
        # ============================================================
        
        # Test 404 error
        response_404 = self.get("/nonexistent.html")
        self.assert_status(response_404, 404)
        self.assert_contains(response_404.text, "Page Not Found")
        
        # ============================================================
        # TEST DIFFERENT HTTP VERSIONS
        # ============================================================
        
        # HTTP/1.1 (default)
        response_11 = self.get("/", http_version="1.1")
        self.assert_status(response_11, 200)
        
        # HTTP/1.0
        response_10 = self.get("/", http_version="1.0")
        self.assert_status(response_10, 200)
        
        # ============================================================
        # TEST POST REQUESTS
        # ============================================================
        
        # Simple POST with form data
        post_data = "username=testuser&password=secret"
        response_post = self.post(
            "/upload",
            data=post_data,
            headers={"Content-Type": "application/x-www-form-urlencoded"}
        )
        # Check if post was accepted (200, 201, or 204 are all acceptable)
        if response_post:
            assert response_post.status_code in [200, 201, 204, 405], \
                f"POST should be handled, got {response_post.status_code}"
        
        # ============================================================
        # TEST CUSTOM ASSERTIONS
        # ============================================================
        
        # You can also use standard Python assertions
        assert len(response.text) > 0, "Response should not be empty"
        
        # Or combine multiple checks
        self.assert_equal(200, response.status_code)
        assert "html" in response.text.lower()


# ============================================================
# TIPS FOR WRITING TESTS
# ============================================================

"""
1. Keep tests focused:
   - Each test should verify one feature or behavior
   - Don't try to test everything in one test

2. Use descriptive names:
   - get_name() should be unique and descriptive
   - get_description() should explain what is being tested

3. Organize resources logically:
   - Group related files in subdirectories
   - Use meaningful filenames

4. Use test helpers:
   - HTMLBuilder for consistent HTML generation
   - CGIBuilder for CGI scripts
   - ConfigBuilder for common configurations

5. Test edge cases:
   - Invalid paths (404 errors)
   - Different HTTP methods
   - Large files
   - Special characters in URLs

6. Assert clearly:
   - Use descriptive assertion messages
   - Check both status codes and content
   - Verify the most important aspects first

7. Handle expected failures:
   - Some responses might legitimately fail
   - Use appropriate assertions (e.g., 405 Method Not Allowed)

8. Debug failed tests:
   - Check generated config: test_runs/test_name/conf/
   - Check generated files: test_runs/test_name/www/
   - Server logs may be in test_runs/test_name/logs/
"""
