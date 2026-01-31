"""
Test: Three servers handling concurrent requests
Description: Tests three servers on different ports (8080, 8081, 8082) handling requests concurrently.
"""

from luka.base_test import BaseTest


class ThreeServersConcurrentTest(BaseTest):
    def get_name(self) -> str:
        return "multi_server_three_concurrent"

    def get_description(self) -> str:
        return (
            "Tests three servers handling concurrent requests on ports 8080, 8081, 8082"
        )

    def setup(self):
        # Create content for three servers
        self.create_html_file("index.html", "<h1>Server 1</h1><p>Port 8080</p>")

        self.create_directory("server2", parent="www")
        self.create_html_file(
            "index.html", "<h1>Server 2</h1><p>Port 8081</p>", subdir="server2"
        )

        self.create_directory("server3", parent="www")
        self.create_html_file(
            "index.html", "<h1>Server 3</h1><p>Port 8082</p>", subdir="server3"
        )

        # Configuration with three servers
        conf = f"""http {{
			server {{
				listen 8080;
				server_name server1.local;
				root {self.www_dir};
				index index.html;
			}}
			
			server {{
				listen 8081;
				server_name server2.local;
				root {self.www_dir}/server2;
				index index.html;
			}}
			
			server {{
				listen 8082;
				server_name server3.local;
				root {self.www_dir}/server3;
				index index.html;
			}}
		}}"""

        self.create_conf(conf)

    def run(self):
        # Test all servers sequentially first
        print("Testing servers sequentially...")

        self.set_port(8080)
        r1 = self.get("/")
        self.assert_status(r1, 200)
        assert b"Server 1" in r1.content
        print("✓ Server 1 (8080) responds")

        self.set_port(8081)
        r2 = self.get("/")
        self.assert_status(r2, 200)
        assert b"Server 2" in r2.content
        print("✓ Server 2 (8081) responds")

        self.set_port(8082)
        r3 = self.get("/")
        self.assert_status(r3, 200)
        assert b"Server 3" in r3.content
        print("✓ Server 3 (8082) responds")

        # Now test concurrent requests to all three servers
        print("\nTesting concurrent requests to all three servers...")

        urls = [
            f"http://localhost:8080/",
            f"http://localhost:8081/",
            f"http://localhost:8082/",
            f"http://localhost:8080/",  # Request to server 1 again
            f"http://localhost:8081/",  # Request to server 2 again
            f"http://localhost:8082/",  # Request to server 3 again
        ]

        responses = self.concurrent_get(urls)

        # Verify all concurrent requests succeeded
        assert len(responses) == 6, f"Expected 6 responses, got {len(responses)}"

        for i, response in enumerate(responses):
            self.assert_status(response, 200, f"Request {i + 1} should succeed")

        # Verify correct content from each server
        assert b"Server 1" in responses[0].content
        assert b"Server 2" in responses[1].content
        assert b"Server 3" in responses[2].content
        assert b"Server 1" in responses[3].content
        assert b"Server 2" in responses[4].content
        assert b"Server 3" in responses[5].content

        print("✓ All 6 concurrent requests handled correctly")
        print("✓ Each server served correct content under concurrent load")
