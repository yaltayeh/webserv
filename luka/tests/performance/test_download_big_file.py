"""
Big file download tests.
Tests server's ability to serve large files correctly.
Tests three different file sizes: 10MB, 50MB, and 100MB.
"""

import os
from luka.base_test import BaseTest
from luka.utilities.test_helpers import FileBuilder, HTMLBuilder
from luka.utilities import http as http_module


class DownloadBigFile10MBTest(BaseTest):
    def get_name(self) -> str:
        return "download_big_file_10mb"

    def get_description(self) -> str:
        return "Tests downloading a 10MB file."

    def setup(self):
        # Create a simple index page
        self.create_html_file(
            "index.html", HTMLBuilder.index_page("Big File Download Test")
        )

        # Create 10MB file
        file_path = os.path.join(self.www_dir, "bigfile_10mb.bin")
        FileBuilder.create_large_file(file_path, size_mb=10, content_type="random")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 50M;
        
        location / {{
            allowed_methods GET HEAD;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Calculate dynamic timeout (10MB file)
        timeout = http_module.calculate_timeout(10)

        # Download the file
        response = self.get("/bigfile_10mb.bin", timeout=timeout)

        # Verify response
        self.assert_status(response, 200, "Should successfully download 10MB file")

        # Verify file size (10MB = 10 * 1024 * 1024 bytes)
        expected_size = 10 * 1024 * 1024
        actual_size = (
            len(response.content)
            if hasattr(response, "content")
            else len(response.text.encode())
        )

        # Allow small variance for headers/encoding
        size_diff = abs(expected_size - actual_size)
        self.assert_equal(
            True,
            size_diff < 1024,  # Within 1KB tolerance
            f"Downloaded file size should be ~10MB (expected {expected_size}, got {actual_size}, diff {size_diff})",
        )


class DownloadBigFile50MBTest(BaseTest):
    def get_name(self) -> str:
        return "download_big_file_50mb"

    def get_description(self) -> str:
        return "Tests downloading a 50MB file."

    def setup(self):
        # Create a simple index page
        self.create_html_file(
            "index.html", HTMLBuilder.index_page("Big File Download Test - 50MB")
        )

        # Create 50MB file
        file_path = os.path.join(self.www_dir, "bigfile_50mb.bin")
        FileBuilder.create_large_file(file_path, size_mb=50, content_type="random")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 100M;
        
        location / {{
            allowed_methods GET HEAD;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Calculate dynamic timeout (50MB file)
        timeout = http_module.calculate_timeout(50)

        # Download the file
        response = self.get("/bigfile_50mb.bin", timeout=timeout)

        # Verify response
        self.assert_status(response, 200, "Should successfully download 50MB file")

        # Verify file size (50MB = 50 * 1024 * 1024 bytes)
        expected_size = 50 * 1024 * 1024
        actual_size = (
            len(response.content)
            if hasattr(response, "content")
            else len(response.text.encode())
        )

        # Allow small variance for headers/encoding
        size_diff = abs(expected_size - actual_size)
        self.assert_equal(
            True,
            size_diff < 1024,  # Within 1KB tolerance
            f"Downloaded file size should be ~50MB (expected {expected_size}, got {actual_size}, diff {size_diff})",
        )


class DownloadBigFile100MBTest(BaseTest):
    def get_name(self) -> str:
        return "download_big_file_100mb"

    def get_description(self) -> str:
        return "Tests downloading a 100MB file."

    def setup(self):
        # Create a simple index page
        self.create_html_file(
            "index.html", HTMLBuilder.index_page("Big File Download Test - 100MB")
        )

        # Create 100MB file
        file_path = os.path.join(self.www_dir, "bigfile_100mb.bin")
        FileBuilder.create_large_file(file_path, size_mb=100, content_type="random")

        # Create configuration
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        client_max_body_size 200M;
        
        location / {{
            allowed_methods GET HEAD;
        }}
    }}
}}"""
        self.create_conf(conf)

    def run(self):
        # Calculate dynamic timeout (100MB file)
        timeout = http_module.calculate_timeout(100)

        # Download the file
        response = self.get("/bigfile_100mb.bin", timeout=timeout)

        # Verify response
        self.assert_status(response, 200, "Should successfully download 100MB file")

        # Verify file size (100MB = 100 * 1024 * 1024 bytes)
        expected_size = 100 * 1024 * 1024
        actual_size = (
            len(response.content)
            if hasattr(response, "content")
            else len(response.text.encode())
        )

        # Allow small variance for headers/encoding
        size_diff = abs(expected_size - actual_size)
        self.assert_equal(
            True,
            size_diff < 1024,  # Within 1KB tolerance
            f"Downloaded file size should be ~100MB (expected {expected_size}, got {actual_size}, diff {size_diff})",
        )
