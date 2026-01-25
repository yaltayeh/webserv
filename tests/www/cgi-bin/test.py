#!/usr/bin/env python3
"""
CGI Test Script
Demonstrates CGI functionality with environment variables and POST data
"""

import os
import sys
from urllib.parse import parse_qs

# Print CGI headers
print("Content-Type: text/html\r")
print("\r")

# HTML start
print("<!DOCTYPE html>")
print("<html><head><title>CGI Test</title>")
print("<style>")
print("body { font-family: Arial; max-width: 800px; margin: 50px auto; padding: 20px; }")
print("h1 { color: #2196F3; }")
print(".section { background: #f5f5f5; padding: 15px; margin: 10px 0; border-radius: 5px; }")
print("pre { background: white; padding: 10px; overflow-x: auto; }")
print("</style>")
print("</head><body>")

print("<h1>🐍 CGI Test Script</h1>")

# Request method
method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
print(f"<div class='section'><h2>Request Method</h2><p>{method}</p></div>")

# Query string
query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print("<div class='section'><h2>Query String</h2>")
    print(f"<pre>{query_string}</pre>")
    params = parse_qs(query_string)
    print("<h3>Parsed Parameters:</h3><ul>")
    for key, values in params.items():
        for value in values:
            print(f"<li><strong>{key}</strong>: {value}</li>")
    print("</ul></div>")

# POST data
if method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print("<div class='section'><h2>POST Data</h2>")
        print(f"<pre>{post_data}</pre>")
        
        # Try to parse as form data
        try:
            params = parse_qs(post_data)
            print("<h3>Parsed POST Parameters:</h3><ul>")
            for key, values in params.items():
                for value in values:
                    print(f"<li><strong>{key}</strong>: {value}</li>")
            print("</ul>")
        except:
            pass
        print("</div>")

# Environment variables
print("<div class='section'><h2>CGI Environment Variables</h2><pre>")
interesting_vars = [
    'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH',
    'SERVER_SOFTWARE', 'SERVER_NAME', 'SERVER_PORT', 'SERVER_PROTOCOL',
    'PATH_INFO', 'PATH_TRANSLATED', 'SCRIPT_NAME', 'SCRIPT_FILENAME',
    'REMOTE_ADDR', 'REMOTE_HOST', 'HTTP_HOST', 'HTTP_USER_AGENT',
    'HTTP_ACCEPT', 'HTTP_COOKIE'
]

for var in interesting_vars:
    value = os.environ.get(var, '(not set)')
    print(f"{var}: {value}")

print("</pre></div>")

# Test examples
print("<div class='section'><h2>Test Examples</h2>")
print("<h3>GET with Query String:</h3>")
print("<pre>curl 'http://localhost:8080/cgi-bin/test.py?name=John&age=25'</pre>")
print("<h3>POST with Data:</h3>")
print("<pre>curl -X POST -d 'name=John&age=25' http://localhost:8080/cgi-bin/test.py</pre>")
print("</div>")

print("</body></html>")
