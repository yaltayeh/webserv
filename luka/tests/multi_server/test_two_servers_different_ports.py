"""
Test: Multiple servers on different ports
Description: Verifies that webserv can run multiple server blocks on different ports simultaneously.
"""

from luka.base_test import BaseTest


class TwoServersDifferentPortsTest(BaseTest):
    def get_name(self) -> str:
        return "multi_server_two_servers_different_ports"

    def get_description(self) -> str:
        return "Tests two servers running on different ports (8080 and 8081)"

    def setup(self):
        # Create content for first server
        self.create_html_file("index.html", "<h1>Server on 8080</h1>")

        # Create subdirectory for second server
        self.create_directory("server2", parent="www")
        self.create_html_file("index.html", "<h1>Server on 8081</h1>", subdir="server2")

        # Configuration with two servers on different ports
        conf = f"""http {{
			server {{
				listen 8080;
				server_name localhost;
				root {self.www_dir};
				index index.html;
			}}
			
			server {{
				listen 8081;
				server_name localhost;
				root {self.www_dir}/server2;
				index index.html;
			}}
		}}"""

        self.create_conf(conf)

    def run(self):
        # Test first server on port 8080
        self.set_port(8080)
        response1 = self.get("/")
        self.assert_status(response1, 200)
        assert b"Server on 8080" in response1.content, (
            "First server should serve its content"
        )
        print("✓ Server on port 8080 is working")

        # Test second server on port 8081
        self.set_port(8081)
        response2 = self.get("/")
        self.assert_status(response2, 200)
        assert b"Server on 8081" in response2.content, (
            "Second server should serve its content"
        )
        print("✓ Server on port 8081 is working")

        # Verify servers are truly different
        assert response1.content != response2.content, (
            "Servers should serve different content"
        )
        print("✓ Both servers serve different content")
