"""
Concurrent downloads test.
Tests downloading a big file while simultaneously handling multiple simple GET requests.
"""

import os
from luka.base_test import BaseTest
from luka.utilities.test_helpers import FileBuilder, HTMLBuilder
from luka.utilities import http as http_module


class ConcurrentDownloadsTest(BaseTest):
    def get_name(self) -> str:
        return "concurrent_downloads"

    def get_description(self) -> str:
        return "Tests downloading 50MB file while 20 users request simple HTML simultaneously."

    def setup(self):
        # Create a simple HTML file for concurrent requests
        simple_html = HTMLBuilder.simple_page(
            title="Simple Page",
            body="<h1>Fast Response</h1><p>This is a small, simple page for concurrent testing.</p>",
        )
        self.create_html_file("simple.html", simple_html)

        # Create index page
        self.create_html_file(
            "index.html", HTMLBuilder.index_page("Concurrent Download Test")
        )

        # Create 50MB file for big download
        big_file_path = os.path.join(self.www_dir, "bigfile_50mb.bin")
        FileBuilder.create_large_file(big_file_path, size_mb=50, content_type="random")

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
        # Calculate dynamic timeout for 50MB file
        timeout = http_module.calculate_timeout(50)

        # Prepare URLs for concurrent requests
        big_file_url = self.get_url("/bigfile_50mb.bin")
        simple_urls = [self.get_url("/simple.html") for _ in range(20)]

        # Combine into one list: 1 big file + 20 simple requests
        all_urls = [big_file_url] + simple_urls

        # Make all requests concurrently
        # This uses threading to download the big file and hit simple.html 20 times in parallel
        results = self.concurrent_get(all_urls, max_workers=21, timeout=timeout)

        # Verify all requests succeeded
        for idx, result in enumerate(results):
            if result.error:
                raise AssertionError(f"Request {idx} failed with error: {result.error}")

            self.assert_equal(True, result.success, f"Request {idx} should succeed")

            self.assert_status(result.response, 200, f"Request {idx} should return 200")

        # Verify the big file (first result) has correct size
        big_file_result = results[0]
        expected_size = 50 * 1024 * 1024
        actual_size = (
            len(big_file_result.response.content)
            if hasattr(big_file_result.response, "content")
            else len(big_file_result.response.text.encode())
        )

        size_diff = abs(expected_size - actual_size)
        self.assert_equal(
            True,
            size_diff < 1024,  # Within 1KB tolerance
            f"Big file should be ~50MB (expected {expected_size}, got {actual_size})",
        )

        # Verify simple HTML responses contain expected content (skip first result which is big file)
        for idx in range(1, len(results)):
            simple_result = results[idx]
            self.assert_contains(
                simple_result.response.text,
                "Fast Response",
                f"Simple HTML request {idx} should contain expected content",
            )

        # Check that simple requests completed relatively quickly
        # Even with a 50MB download happening, simple requests shouldn't take too long
        simple_durations = [results[i].duration for i in range(1, len(results))]
        max_simple_duration = max(simple_durations) if simple_durations else 0

        # Simple requests should complete in reasonable time even with concurrent big download
        # Allow up to 10 seconds for simple requests (generous timeout)
        self.assert_equal(
            True,
            max_simple_duration < 10.0,
            f"Simple requests should be relatively fast even with concurrent download (slowest was {max_simple_duration:.2f}s)",
        )

        # Print timing information for debugging
        big_file_duration = results[0].duration
        avg_simple_duration = (
            sum(simple_durations) / len(simple_durations) if simple_durations else 0
        )
        print(f"  Big file download: {big_file_duration:.2f}s")
        print(
            f"  Simple requests: avg {avg_simple_duration:.2f}s, max {max_simple_duration:.2f}s"
        )
