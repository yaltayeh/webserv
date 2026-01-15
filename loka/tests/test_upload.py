"""
File upload test.
Tests if the server handles file uploads correctly.
"""
from loka.base_test import BaseTest
from loka.utilities.test_helpers import HTMLBuilder
import os


class FileUploadTest(BaseTest):
    def get_name(self) -> str:
        return "file_upload"
    
    def get_description(self) -> str:
        return "Tests if the server handles file uploads via POST/PUT."
    
    def setup(self):
        # Create upload form page
        self.create_html_file("index.html", HTMLBuilder.form_page("/upload", "POST"))
        
        # Create configuration with upload support
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 10M;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /upload {{
            allowed_methods POST PUT;
            upload_enable on;
            upload_store {self.uploads_dir};
        }}
    }}
}}"""
        self.create_conf(conf)
    
    def run(self):
        # Test GET request to form page
        response = self.get("/")
        self.assert_status(response, 200)
        self.assert_contains(response.text, "Upload", "Should display upload form")
        
        # Test file upload via POST
        # Note: This is a simplified test. In reality, you'd need multipart/form-data
        file_content = "This is test file content"
        response_upload = self.post("/upload", 
                                    data=file_content,
                                    headers={"Content-Type": "text/plain"})
        
        # Server should accept the upload (status codes 200, 201, or 204 are acceptable)
        if response_upload:
            assert response_upload.status_code in [200, 201, 204], \
                f"Upload should succeed, got {response_upload.status_code}"
