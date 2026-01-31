"""
Test: Virtual host routing by server_name
Description: Tests that requests are routed to the correct server based on the Host header.
"""

from luka.base_test import BaseTest


class VirtualHostRoutingTest(BaseTest):
    def get_name(self) -> str:
        return "multi_server_virtual_host_routing"

    def get_description(self) -> str:
        return "Tests virtual host routing based on Host header (server_name directive)"

    def setup(self):
        # Create different content for each virtual host
        self.create_directory("site1", parent="www")
        self.create_html_file(
            "index.html", "<h1>Site 1</h1><p>www.site1.local</p>", subdir="site1"
        )

        self.create_directory("site2", parent="www")
        self.create_html_file(
            "index.html", "<h1>Site 2</h1><p>www.site2.local</p>", subdir="site2"
        )

        self.create_directory("site3", parent="www")
        self.create_html_file(
            "index.html", "<h1>Site 3</h1><p>www.site3.local</p>", subdir="site3"
        )

        # Three virtual hosts on the SAME port with different server_names
        conf = f"""http {{
			server {{
				listen 8080;
				server_name www.site1.local site1.local;
				root {self.www_dir}/site1;
				index index.html;
			}}
			
			server {{
				listen 8080;
				server_name www.site2.local site2.local;
				root {self.www_dir}/site2;
				index index.html;
			}}
			
			server {{
				listen 8080;
				server_name www.site3.local site3.local;
				root {self.www_dir}/site3;
				index index.html;
			}}
		}}"""

        self.create_conf(conf)

    def run(self):
        self.set_port(8080)

        print("Testing virtual host routing on same port (8080)...")

        # Request to site1 with Host: www.site1.local
        r1 = self.get("/", headers={"Host": "www.site1.local"})
        self.assert_status(r1, 200)
        assert b"Site 1" in r1.content, "Should serve Site 1 content"
        assert b"www.site1.local" in r1.content
        print("✓ Virtual host 'www.site1.local' routes correctly")

        # Request to site2 with Host: www.site2.local
        r2 = self.get("/", headers={"Host": "www.site2.local"})
        self.assert_status(r2, 200)
        assert b"Site 2" in r2.content, "Should serve Site 2 content"
        assert b"www.site2.local" in r2.content
        print("✓ Virtual host 'www.site2.local' routes correctly")

        # Request to site3 with Host: www.site3.local
        r3 = self.get("/", headers={"Host": "www.site3.local"})
        self.assert_status(r3, 200)
        assert b"Site 3" in r3.content, "Should serve Site 3 content"
        assert b"www.site3.local" in r3.content
        print("✓ Virtual host 'www.site3.local' routes correctly")

        # Test alternate names (without www)
        r4 = self.get("/", headers={"Host": "site1.local"})
        self.assert_status(r4, 200)
        assert b"Site 1" in r4.content
        print("✓ Alternate name 'site1.local' routes correctly")

        # Verify responses are different
        assert r1.content != r2.content, "Site 1 and Site 2 should serve different content"
        assert r2.content != r3.content, "Site 2 and Site 3 should serve different content"
        assert r1.content != r3.content, "Site 1 and Site 3 should serve different content"

        print("✓ All virtual hosts serve unique content based on Host header")

        # Test with unknown host (should route to first/default server)
        print("\nTesting unknown Host header...")
        r5 = self.get("/", headers={"Host": "unknown.local"})
        self.assert_status(r5, 200)
        print(f"✓ Unknown host handled (routed to default server)")
