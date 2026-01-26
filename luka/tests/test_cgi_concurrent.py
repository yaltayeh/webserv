"""
CGI concurrent load test.
Tests that 30 concurrent slow CGI requests don't block simple GET requests.
"""

import time
from luka.base_test import BaseTest
from luka.utilities.test_helpers import HTMLBuilder, CGIBuilder


class CGIConcurrentTest(BaseTest):
    def get_name(self) -> str:
        return "cgi_concurrent"

    def get_description(self) -> str:
        return "Tests that 30 slow CGI requests don't block a simple GET request."

    def setup(self):
        # Create a simple HTML file for the quick GET request
        simple_html = HTMLBuilder.simple_page(
            title="Simple Page",
            body="<h1>Quick Response</h1><p>This should load quickly even when CGI is busy.</p>",
        )
        self.create_html_file("quick.html", simple_html)

        # Create index page
        self.create_html_file(
            "index.html", HTMLBuilder.index_page("CGI Concurrent Test")
        )

        # Create slow CGI script (10 second sleep)
        slow_cgi = CGIBuilder.python_sleep(10)
        self.create_cgi_script("slow.py", slow_cgi)

        # Create configuration with CGI support
        conf = f"""http {{
    server {{
        listen {self.port};
        server_name localhost;
        root {self.www_dir};
        index index.html;
        
        location / {{
            allowed_methods GET HEAD;
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
        # Prepare request list:
        # - 30 CGI requests (slow, 10 seconds each)
        # - 1 simple HTML request (should be fast)

        # Create URLs for CGI requests
        cgi_urls = [self.get_url("/cgi-bin/slow.py") for _ in range(30)]

        # Create URL for simple request
        simple_url = self.get_url("/quick.html")

        # Start timing
        start_time = time.time()

        # First, launch all 30 CGI requests in parallel
        # Then immediately make the simple request
        # The simple request should complete quickly despite 30 slow CGI running

        all_urls = cgi_urls + [simple_url]

        # Make all requests concurrently (30 CGI + 1 simple)
        # Use 15 second timeout since CGI scripts sleep for 10 seconds
        results = self.concurrent_get(all_urls, max_workers=31, timeout=15)

        total_time = time.time() - start_time

        # Verify all requests succeeded
        for idx, result in enumerate(results):
            self.assert_equal(
                True,
                result.success,
                f"Request {idx} should succeed (error: {result.error})",
            )

            self.assert_status(result.response, 200, f"Request {idx} should return 200")

        # Get timing for CGI requests (first 30 results)
        cgi_results = results[:30]
        cgi_durations = [r.duration for r in cgi_results]

        # Get timing for simple request (last result)
        simple_result = results[30]
        simple_duration = simple_result.duration

        # The key assertion: simple request should complete MUCH faster than CGI requests
        # Even though 30 CGI scripts are running with 10-second sleeps,
        # the simple GET should complete in < 2 seconds
        self.assert_equal(
            True,
            simple_duration < 2.0,
            f"Simple GET should complete quickly (< 2s) even with 30 concurrent CGI requests. Actual: {simple_duration:.2f}s",
        )

        # Verify CGI requests took approximately 10 seconds (they have sleep(10))
        # Allow some variance for processing time
        avg_cgi_duration = sum(cgi_durations) / len(cgi_durations)
        self.assert_equal(
            True,
            8.0 < avg_cgi_duration < 15.0,
            f"CGI requests should take ~10s due to sleep (got avg {avg_cgi_duration:.2f}s)",
        )

        # Verify simple HTML response contains expected content
        self.assert_contains(
            simple_result.response.text,
            "Quick Response",
            "Simple HTML response should contain expected content",
        )

        # Verify CGI responses contain expected content
        for idx, cgi_result in enumerate(
            cgi_results[:3]
        ):  # Check first 3 CGI responses
            self.assert_contains(
                cgi_result.response.text,
                "Slow CGI Response",
                f"CGI response {idx} should contain expected content",
            )

        # Print timing summary for debugging
        print(f"  Total test time: {total_time:.2f}s")
        print(f"  Simple GET duration: {simple_duration:.2f}s")
        print(
            f"  CGI avg duration: {avg_cgi_duration:.2f}s (min: {min(cgi_durations):.2f}s, max: {max(cgi_durations):.2f}s)"
        )
        print(
            f"  SUCCESS: Server remained responsive during 30 concurrent slow CGI requests"
        )
