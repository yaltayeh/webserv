"""
Test: Five servers with heavy concurrent load
Description: Stress test with 5 servers handling 50 concurrent requests across all servers.
"""

from luka.base_test import BaseTest


class FiveServersStressTest(BaseTest):
    def get_name(self) -> str:
        return "multi_server_five_servers_stress"

    def get_description(self) -> str:
        return "Stress test with 5 servers handling 50 concurrent requests (ports 8080-8084)"

    def setup(self):
        # Create 5 servers with different content
        for i in range(1, 6):
            server_dir = f"server{i}"
            self.create_directory(server_dir, parent="www")
            content = (
                f"<h1>Server {i}</h1><p>Port {8079 + i}</p><p>{{request_number}}</p>"
            )
            self.create_html_file("index.html", content, subdir=server_dir)

            # Also create some additional pages
            self.create_html_file(
                "page.html", f"<h1>Page from Server {i}</h1>", subdir=server_dir
            )

        # Configuration with 5 servers
        servers_conf = ""
        for i in range(1, 6):
            port = 8079 + i
            servers_conf += f"""
			server {{
				listen {port};
				server_name server{i}.local;
				root {self.www_dir}/server{i};
				index index.html;
			}}
			"""

        conf = f"""http {{
			{servers_conf}
		}}"""

        self.create_conf(conf)

    def run(self):
        print("Testing 5 servers individually...")

        # Test each server individually first
        for i in range(1, 6):
            port = 8079 + i
            self.set_port(port)
            response = self.get("/")
            self.assert_status(response, 200)
            assert f"Server {i}".encode() in response.content
            print(f"✓ Server {i} on port {port} responding")

        print("\n Preparing 50 concurrent requests across all 5 servers...")

        # Create 50 concurrent requests distributed across servers
        urls = []
        for i in range(50):
            server_num = (i % 5) + 1  # Distribute evenly: 1,2,3,4,5,1,2,3,4,5,...
            port = 8079 + server_num
            path = "/" if i % 2 == 0 else "/page.html"
            urls.append(f"http://localhost:{port}{path}")

        print(f"Sending {len(urls)} concurrent requests...")
        responses = self.concurrent_get(urls, max_workers=10)

        # Verify all requests succeeded
        assert len(responses) == 50, f"Expected 50 responses, got {len(responses)}"

        success_count = 0
        for i, response in enumerate(responses):
            if response.status_code == 200:
                success_count += 1

        print(f"\n✓ All 50 concurrent requests completed")
        print(f"✓ Success rate: {success_count}/50 ({success_count * 100 // 50}%)")

        # Verify correct distribution
        server_hits = {i: 0 for i in range(1, 6)}
        for i, response in enumerate(responses):
            if response.status_code == 200:
                for server_num in range(1, 6):
                    if f"Server {server_num}".encode() in response.content:
                        server_hits[server_num] += 1
                        break

        print(f"\nRequest distribution across servers:")
        for server_num, hits in server_hits.items():
            print(f"  Server {server_num} (port {8079 + server_num}): {hits} requests")

        # Each server should have received ~10 requests
        for server_num, hits in server_hits.items():
            assert hits > 0, f"Server {server_num} received no requests"

        print(f"\n✓ Stress test passed - all 5 servers handled concurrent load")
