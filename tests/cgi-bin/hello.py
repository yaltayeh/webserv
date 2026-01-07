#!/usr/bin/env python3
"""Simple CGI script that returns Hello World"""
import os

print("Content-Type: text/html")
print("X-Custom-Header: test-value")
print()
print("<html>")
print("<head><title>Hello CGI</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("<p>REQUEST_METHOD: {}</p>".format(os.environ.get('REQUEST_METHOD', 'N/A')))
print("<p>QUERY_STRING: {}</p>".format(os.environ.get('QUERY_STRING', 'N/A')))
print("<p>SCRIPT_NAME: {}</p>".format(os.environ.get('SCRIPT_NAME', 'N/A')))
print("</body>")
print("</html>")
