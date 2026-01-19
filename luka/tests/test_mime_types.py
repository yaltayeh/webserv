"""
MIME types test.
Tests that the server returns correct Content-Type headers for different file types (HTTP/1.0).
"""

from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder


class MimeTypesTest(BaseTest):
    def get_name(self) -> str:
        return "mime_types"

    def get_description(self) -> str:
        return "Tests that server returns correct Content-Type for various file extensions (HTTP/1.0)."

    def setup(self):
        # Create files with different extensions
        self.create_html_file("index.html", HTMLBuilder.index_page("MIME Test"))
        self.create_html_file(
            "page.htm", HTMLBuilder.simple_page("HTM", "<h1>HTM file</h1>")
        )

        # Text files
        self.create_file("data.txt", "Plain text content")
        self.create_file("style.css", "body { background: #fff; }")
        self.create_file("script.js", "console.log('test');")
        self.create_file("data.json", '{"key": "value"}')
        self.create_file("data.xml", '<?xml version="1.0"?><root></root>')

        # Create subdirectory with various files
        self.create_directory("assets")
        self.create_file("readme.md", "# Markdown file", subdir="assets")
        self.create_file("data.csv", "name,value\ntest,123", subdir="assets")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Test HTML files
        response_html = self.get("/index.html")
        self.assert_status(response_html, 200, "HTML file should be accessible")
        if (
            hasattr(response_html, "headers")
            and "Content-Type" in response_html.headers
        ):
            content_type = response_html.headers["Content-Type"].lower()
            assert "text/html" in content_type, (
                f"HTML file should have text/html Content-Type, got {content_type}"
            )

        # Test HTM files
        response_htm = self.get("/page.htm")
        self.assert_status(response_htm, 200, "HTM file should be accessible")

        # Test plain text
        response_txt = self.get("/data.txt")
        self.assert_status(response_txt, 200, "Text file should be accessible")
        if hasattr(response_txt, "headers") and "Content-Type" in response_txt.headers:
            content_type = response_txt.headers["Content-Type"].lower()
            assert "text/plain" in content_type or "text/" in content_type, (
                f"Text file should have text/plain Content-Type, got {content_type}"
            )

        # Test CSS files
        response_css = self.get("/style.css")
        self.assert_status(response_css, 200, "CSS file should be accessible")
        if hasattr(response_css, "headers") and "Content-Type" in response_css.headers:
            content_type = response_css.headers["Content-Type"].lower()
            # Might be text/css or application/css
            assert "css" in content_type, (
                f"CSS file should have CSS Content-Type, got {content_type}"
            )

        # Test JavaScript files
        response_js = self.get("/script.js")
        self.assert_status(response_js, 200, "JS file should be accessible")
        if hasattr(response_js, "headers") and "Content-Type" in response_js.headers:
            content_type = response_js.headers["Content-Type"].lower()
            # Might be application/javascript, application/x-javascript, or text/javascript
            assert "javascript" in content_type or "js" in content_type, (
                f"JS file should have JavaScript Content-Type, got {content_type}"
            )

        # Test JSON files
        response_json = self.get("/data.json")
        self.assert_status(response_json, 200, "JSON file should be accessible")
        if (
            hasattr(response_json, "headers")
            and "Content-Type" in response_json.headers
        ):
            content_type = response_json.headers["Content-Type"].lower()
            # Should be application/json
            assert "json" in content_type, (
                f"JSON file should have JSON Content-Type, got {content_type}"
            )

        # Test XML files
        response_xml = self.get("/data.xml")
        self.assert_status(response_xml, 200, "XML file should be accessible")

        # Test files in subdirectory
        response_md = self.get("/assets/readme.md")
        self.assert_status(response_md, 200, "Markdown file should be accessible")

        response_csv = self.get("/assets/data.csv")
        self.assert_status(response_csv, 200, "CSV file should be accessible")
