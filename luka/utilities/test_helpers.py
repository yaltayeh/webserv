"""
Helper utilities for creating test resources.
"""

import os
import random


class FileBuilder:
    """Helper class for building test files."""

    @staticmethod
    def create_large_file(filepath: str, size_mb: int, content_type: str = "random"):
        """
        Create a large file for testing downloads.

        Args:
            filepath: Path where file will be created
            size_mb: Size of file in megabytes
            content_type: Type of content ('random', 'text', 'sequential')
                - 'random': Random binary data (not compressible)
                - 'text': Repeated text pattern (compressible)
                - 'sequential': Sequential bytes 0x00-0xFF repeated
        """
        chunk_size = 1024 * 1024  # 1 MB chunks
        bytes_written = 0
        target_bytes = size_mb * 1024 * 1024

        with open(filepath, "wb") as f:
            while bytes_written < target_bytes:
                # Determine how much to write this iteration
                remaining = target_bytes - bytes_written
                this_chunk_size = min(chunk_size, remaining)

                # Generate content based on type
                if content_type == "random":
                    # Random bytes - not compressible
                    chunk = bytes(random.getrandbits(8) for _ in range(this_chunk_size))
                elif content_type == "text":
                    # Repeated text pattern - compressible
                    pattern = b"This is test data for webserv download testing. " * 100
                    chunk = (pattern * (this_chunk_size // len(pattern) + 1))[
                        :this_chunk_size
                    ]
                elif content_type == "sequential":
                    # Sequential bytes - somewhat compressible
                    chunk = bytes((i % 256) for i in range(this_chunk_size))
                else:
                    raise ValueError(f"Unknown content_type: {content_type}")

                f.write(chunk)
                bytes_written += len(chunk)

    @staticmethod
    def create_binary_file(filepath: str, size_mb: int):
        """
        Create a binary file with random data (alias for create_large_file with random content).

        Args:
            filepath: Path where file will be created
            size_mb: Size of file in megabytes
        """
        FileBuilder.create_large_file(filepath, size_mb, content_type="random")


class HTMLBuilder:
    """Helper class for building HTML content."""

    @staticmethod
    def simple_page(title: str, body: str) -> str:
        """Create a simple HTML page."""
        return f"""<!DOCTYPE html>
<html>
<head>
    <title>{title}</title>
</head>
<body>
    {body}
</body>
</html>"""

    @staticmethod
    def index_page(heading: str = "Welcome") -> str:
        """Create a basic index page."""
        return f"""<!DOCTYPE html>
<html>
<head>
    <title>Index</title>
</head>
<body>
    <h1>{heading}</h1>
    <p>This is the index page.</p>
</body>
</html>"""

    @staticmethod
    def error_page(code: int, message: str) -> str:
        """Create an error page."""
        return f"""<!DOCTYPE html>
<html>
<head>
    <title>Error {code}</title>
</head>
<body>
    <h1>Error {code}</h1>
    <p>{message}</p>
</body>
</html>"""

    @staticmethod
    def form_page(action: str = "/upload", method: str = "POST") -> str:
        """Create a page with a file upload form."""
        return f"""<!DOCTYPE html>
<html>
<head>
    <title>Upload Form</title>
</head>
<body>
    <h1>File Upload</h1>
    <form action="{action}" method="{method}" enctype="multipart/form-data">
        <input type="file" name="file">
        <input type="submit" value="Upload">
    </form>
</body>
</html>"""


class CGIBuilder:
    """Helper class for building CGI scripts."""

    @staticmethod
    def python_hello_world() -> str:
        """Create a simple Python CGI script."""
        return """#!/usr/bin/env python3
import sys

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print("</body></html>")
"""

    @staticmethod
    def python_echo_env() -> str:
        """Create a Python CGI script that echoes environment variables."""
        return """#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Environment Variables</h1>")
print("<ul>")
for key in sorted(os.environ.keys()):
    print(f"<li><strong>{key}:</strong> {os.environ[key]}</li>")
print("</ul>")
print("</body></html>")
"""

    @staticmethod
    def python_query_string() -> str:
        """Create a Python CGI script that processes query strings."""
        return """#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

query_string = os.environ.get('QUERY_STRING', '')
params = parse_qs(query_string)

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Query String Parameters</h1>")
if params:
    print("<ul>")
    for key, values in params.items():
        for value in values:
            print(f"<li><strong>{key}:</strong> {value}</li>")
    print("</ul>")
else:
    print("<p>No query parameters found.</p>")
print("</body></html>")
"""

    @staticmethod
    def python_post_data() -> str:
        """Create a Python CGI script that processes POST data."""
        return """#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

content_length = int(os.environ.get('CONTENT_LENGTH', 0))
post_data = sys.stdin.read(content_length) if content_length > 0 else ''
params = parse_qs(post_data)

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>POST Data</h1>")
if params:
    print("<ul>")
    for key, values in params.items():
        for value in values:
            print(f"<li><strong>{key}:</strong> {value}</li>")
    print("</ul>")
else:
    print("<p>No POST data found.</p>")
print("</body></html>")
"""

    @staticmethod
    def python_sleep(seconds: int) -> str:
        """
        Create a Python CGI script that sleeps for specified seconds.
        Useful for testing server behavior under slow CGI execution.

        Args:
            seconds: Number of seconds to sleep

        Returns:
            CGI script content
        """
        return f"""#!/usr/bin/env python3
import sys
import time

# Sleep for {seconds} seconds to simulate slow processing
time.sleep({seconds})

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Slow CGI Response</h1>")
print(f"<p>This CGI script slept for {seconds} seconds before responding.</p>")
print(f"<p>Current time: {{time.strftime('%Y-%m-%d %H:%M:%S')}}</p>")
print("</body></html>")
"""

    @staticmethod
    def bash_hello_world() -> str:
        """Create a simple Bash CGI script."""
        return """#!/bin/bash

echo "Content-Type: text/html"
echo ""
echo "<html><body>"
echo "<h1>Hello from Bash CGI!</h1>"
echo "</body></html>"
"""

    @staticmethod
    def php_hello_world() -> str:
        """Create a simple PHP script (for FastCGI testing)."""
        return """<?php
header("Content-Type: text/html");
?>
<html>
<body>
    <h1>Hello from PHP!</h1>
    <p>Server time: <?php echo date('Y-m-d H:i:s'); ?></p>
</body>
</html>
"""


class ConfigBuilder:
    """Helper class for building configuration snippets."""

    @staticmethod
    def basic_server(port: int, root: str, server_name: str = "localhost") -> str:
        """Create a basic server configuration."""
        return f"""http {{
    server {{
        listen {port};
        server_name {server_name};
        root {root};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""

    @staticmethod
    def server_with_autoindex(
        port: int, root: str, server_name: str = "localhost"
    ) -> str:
        """Create a server configuration with autoindex enabled."""
        return f"""http {{
    server {{
        listen {port};
        server_name {server_name};
        root {root};
        index index.html;
        autoindex on;
        
        location / {{
            allowed_methods GET POST;
            autoindex on;
        }}
    }}
}}"""

    @staticmethod
    def server_with_cgi(
        port: int, root: str, cgi_dir: str, server_name: str = "localhost"
    ) -> str:
        """Create a server configuration with CGI support."""
        return f"""http {{
    server {{
        listen {port};
        server_name {server_name};
        root {root};
        index index.html;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /cgi-bin/ {{
            allowed_methods GET POST;
            cgi_extension .py .sh;
            cgi_pass {cgi_dir};
        }}
    }}
}}"""

    @staticmethod
    def server_with_upload(
        port: int, root: str, upload_dir: str, server_name: str = "localhost"
    ) -> str:
        """Create a server configuration with upload support."""
        return f"""http {{
    server {{
        listen {port};
        server_name {server_name};
        root {root};
        index index.html;
        client_max_body_size 10M;
        
        location / {{
            allowed_methods GET POST;
        }}
        
        location /upload {{
            allowed_methods POST PUT;
            upload_enable on;
            upload_store {upload_dir};
        }}
    }}
}}"""

    @staticmethod
    def server_with_error_pages(
        port: int, root: str, error_pages: dict, server_name: str = "localhost"
    ) -> str:
        """Create a server configuration with custom error pages."""
        error_page_directives = "\n        ".join(
            f"error_page {code} {page};" for code, page in error_pages.items()
        )
        return f"""http {{
    server {{
        listen {port};
        server_name {server_name};
        root {root};
        index index.html;
        {error_page_directives}
        
        location / {{
            allowed_methods GET POST;
        }}
    }}
}}"""
