#!/usr/bin/env python3
"""CGI script that echoes all environment variables"""
import os

print("Content-Type: text/plain")
print()

# Print all CGI-relevant environment variables
cgi_vars = [
    'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH',
    'SCRIPT_NAME', 'SCRIPT_FILENAME', 'PATH_INFO', 'PATH_TRANSLATED',
    'SERVER_NAME', 'SERVER_PORT', 'SERVER_PROTOCOL', 'SERVER_SOFTWARE',
    'GATEWAY_INTERFACE', 'DOCUMENT_ROOT', 'REQUEST_URI', 'REDIRECT_STATUS'
]

for var in sorted(cgi_vars):
    value = os.environ.get(var, '')
    print("{}={}".format(var, value))

# Print HTTP_* variables
print("\n--- HTTP Headers ---")
for key, value in sorted(os.environ.items()):
    if key.startswith('HTTP_'):
        print("{}={}".format(key, value))
