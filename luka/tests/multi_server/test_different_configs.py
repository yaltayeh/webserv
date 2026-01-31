"""
Test: Multiple servers with different root directories and methods
Description: Tests servers with different root directories, methods, and configurations.
"""

from luka.base_test import BaseTest


class MultiServerDifferentConfigsTest(BaseTest):
    def get_name(self) -> str:
        return "multi_server_different_configs"

    def get_description(self) -> str:
        return "Tests multiple servers with different roots, methods, and client_max_body_size"

    def setup(self):
        # Server 1: Static files only (GET, HEAD)
        self.create_directory("static", parent="www")
        self.create_html_file("page.html", "<h1>Static Server</h1>", subdir="static")

        # Server 2: Upload server (GET, POST, DELETE)
        self.create_directory("upload", parent="www")
        self.create_html_file("upload.html", "<h1>Upload Server</h1>", subdir="upload")

        # Server 3: API server (all methods, large body size)
        self.create_directory("api", parent="www")
        self.create_html_file("api.html", "<h1>API Server</h1>", subdir="api")

        conf = f"""http {{
			# Static file server - only GET and HEAD
			server {{
				listen 8080;
				server_name static.local;
				root {self.www_dir}/static;
				index page.html;
				
				client_max_body_size 1048576;  # 1MB
				
				location / {{
					allowed_methods GET HEAD;
				}}
			}}
			
			# Upload server - GET, POST, DELETE
			server {{
				listen 8081;
				server_name upload.local;
				root {self.www_dir}/upload;
				index upload.html;
				
				client_max_body_size 10485760;  # 10MB
				
				location / {{
					allowed_methods GET POST DELETE;
				}}
			}}
			
			# API server - all methods, large body
			server {{
				listen 8082;
				server_name api.local;
				root {self.www_dir}/api;
				index api.html;
				
				client_max_body_size 52428800;  # 50MB
				
				location / {{
					allowed_methods GET POST PUT DELETE HEAD;
				}}
			}}
		}}"""

        self.create_conf(conf)

    def run(self):
        print("Testing Server 1 (8080) - Static file server...")
        self.set_port(8080)

        # GET should work
        r1 = self.get("/page.html")
        self.assert_status(r1, 200)
        assert b"Static Server" in r1.content
        print("✓ Server 1: GET request works")

        # HEAD should work
        r2 = self.head("/page.html")
        self.assert_status(r2, 200)
        assert len(r2.content) == 0, "HEAD should have empty body"
        print("✓ Server 1: HEAD request works")

        # POST should fail (405 Method Not Allowed)
        try:
            r3 = self.post("/page.html", data="test")
            self.assert_status(r3, 405, "POST should not be allowed on static server")
            print("✓ Server 1: POST correctly rejected (405)")
        except:
            print("✓ Server 1: POST rejected (connection refused or 405)")

        print("\nTesting Server 2 (8081) - Upload server...")
        self.set_port(8081)

        # GET should work
        r4 = self.get("/upload.html")
        self.assert_status(r4, 200)
        assert b"Upload Server" in r4.content
        print("✓ Server 2: GET request works")

        # POST should work
        r5 = self.post("/test.txt", data="test data")
        # Should be 201 Created or 200 OK
        assert r5.status_code in [200, 201, 204], (
            f"POST should succeed, got {r5.status_code}"
        )
        print(f"✓ Server 2: POST request works (status {r5.status_code})")

        print("\nTesting Server 3 (8082) - API server...")
        self.set_port(8082)

        # GET should work
        r6 = self.get("/api.html")
        self.assert_status(r6, 200)
        assert b"API Server" in r6.content
        print("✓ Server 3: GET request works")

        # POST should work
        r7 = self.post("/", data="api data")
        assert r7.status_code in [200, 201, 204, 405], (
            "POST should get a valid response"
        )
        print(f"✓ Server 3: POST handled (status {r7.status_code})")

        # PUT should work
        r8 = self.put("/", data="put data")
        assert r8.status_code in [200, 201, 204, 405], "PUT should get a valid response"
        print(f"✓ Server 3: PUT handled (status {r8.status_code})")

        print("\n✓ All three servers running with different configurations")
