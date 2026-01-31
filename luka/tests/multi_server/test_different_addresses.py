"""
Test: Multiple servers with different listen addresses
Description: Tests servers listening on different addresses (0.0.0.0, 127.0.0.1, localhost).
"""

from luka.base_test import BaseTest


class MultiServerDifferentAddressesTest(BaseTest):
    def get_name(self) -> str:
        return "multi_server_different_addresses"

    def get_description(self) -> str:
        return "Tests servers with different listen addresses (0.0.0.0, 127.0.0.1, explicit host)"

    def setup(self):
        # Create content for each server
        self.create_directory("server1", parent="www")
        self.create_html_file(
            "index.html", "<h1>Server 1</h1><p>0.0.0.0:8080</p>", subdir="server1"
        )

        self.create_directory("server2", parent="www")
        self.create_html_file(
            "index.html", "<h1>Server 2</h1><p>127.0.0.1:8081</p>", subdir="server2"
        )

        self.create_directory("server3", parent="www")
        self.create_html_file(
            "index.html", "<h1>Server 3</h1><p>localhost:8082</p>", subdir="server3"
        )

        # Configuration with different listen addresses
        conf = f"""http {{
			# Listen on all interfaces
			server {{
				listen 0.0.0.0:8080;
				server_name all-interfaces;
				root {self.www_dir}/server1;
				index index.html;
			}}
			
			# Listen on loopback only (explicit IP)
			server {{
				listen 127.0.0.1:8081;
				server_name loopback-explicit;
				root {self.www_dir}/server2;
				index index.html;
			}}
			
			# Listen on localhost (default behavior if no IP specified)
			server {{
				listen 8082;
				server_name loopback-default;
				root {self.www_dir}/server3;
				index index.html;
			}}
		}}"""

        self.create_conf(conf)

    def run(self):
        print("Testing server on 0.0.0.0:8080...")
        self.set_port(8080)
        r1 = self.get("/")
        self.assert_status(r1, 200)
        assert b"Server 1" in r1.content
        assert b"0.0.0.0:8080" in r1.content
        print("✓ Server listening on 0.0.0.0:8080 works")

        print("\nTesting server on 127.0.0.1:8081...")
        self.set_port(8081)
        r2 = self.get("/")
        self.assert_status(r2, 200)
        assert b"Server 2" in r2.content
        assert b"127.0.0.1:8081" in r2.content
        print("✓ Server listening on 127.0.0.1:8081 works")

        print("\nTesting server on port 8082 (default localhost)...")
        self.set_port(8082)
        r3 = self.get("/")
        self.assert_status(r3, 200)
        assert b"Server 3" in r3.content
        assert b"localhost:8082" in r3.content
        print("✓ Server listening on 8082 (localhost) works")

        print("\n✓ All three servers with different listen addresses work correctly")
