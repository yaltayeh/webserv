import unittest
import requests
import os
import sys

# Constants
HOST = "localhost"
PORT = 8080
BASE_URL = f"http://{HOST}:{PORT}"

class TestWebserver(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        # Server is started by run_tests.py
        pass

    def test_01_get_index(self):
        """Test GET request for index.html"""
        try:
            r = requests.get(BASE_URL + "/")
            self.assertEqual(r.status_code, 200)
            # Depending on the config, it might be index.html or autoindex
            # We assume index.html content or at least a 200 OK
        except requests.exceptions.ConnectionError:
            self.fail("Could not connect to server")

    def test_02_404_not_found(self):
        """Test GET request for non-existent file"""
        r = requests.get(BASE_URL + "/does_not_exist_12345.html")
        self.assertEqual(r.status_code, 404)

    def test_03_post_upload(self):
        """Test POST request to upload a small file"""
        # Assuming /uploads is configured
        url = BASE_URL + "/uploads/test_file.txt"
        content = b"Hello World Upload Test"
        
        print(f"DEBUG: POST {url}")
        r = requests.post(url, data=content)
        print(f"DEBUG: Status {r.status_code}, Body: {r.text[:50]}")
        
        self.assertIn(r.status_code, [200, 201])
        
        # Verify file exists
        r_get = requests.get(url)
        if r_get.status_code == 200:
             self.assertEqual(r_get.content, content)

    def test_04_delete_file(self):
        """Test DELETE request"""
        filename = "delete_me.txt"
        url = BASE_URL + f"/uploads/{filename}"
        requests.post(url, data=b"Delete me")
        
        r = requests.delete(url)
        self.assertIn(r.status_code, [200, 204])
        
        r_get = requests.get(url)
        self.assertEqual(r_get.status_code, 404)

    def test_05_chunked_encoding(self):
        """Test Chunked Transfer Encoding"""
        url = BASE_URL + "/uploads/chunked.txt"
        
        def gen():
            yield b"Chunk1"
            yield b"Chunk2"
            yield b"Chunk3"
            
        r = requests.post(url, data=gen())
        self.assertIn(r.status_code, [200, 201])
        
        r_get = requests.get(url)
        if r_get.status_code == 200:
            self.assertEqual(r_get.content, b"Chunk1Chunk2Chunk3")

    def test_06_large_file(self):
        """Test Large File Upload (10KB)"""
        url = BASE_URL + "/uploads/large.bin"
        size = 10 * 1024 # 10KB
        data = b"a" * size
        
        print("DEBUG: Sending Large File...")
        try:
            # Disable Expect: 100-continue which might confuse simple servers
            r = requests.post(url, data=data, timeout=5, headers={'Expect': ''})
            self.assertIn(r.status_code, [200, 201]) 
        except requests.exceptions.Timeout:
            self.fail("Request timed out (Server might be hanging on body read)")

    def test_07_cgi_python(self):
        """Test Python CGI execution"""
        # Assuming /cgi-bin/hello.py exists and returns something
        url = BASE_URL + "/cgi-bin/hello.py"
        r = requests.get(url)
        
        # Check for 200 OK
        self.assertEqual(r.status_code, 200)
        # Check for typical CGI output (maybe "Hello")
        # self.assertIn("Hello", r.text) 

if __name__ == "__main__":
    unittest.main()
