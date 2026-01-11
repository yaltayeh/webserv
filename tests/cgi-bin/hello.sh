#!/bin/bash
# Simple shell CGI script

echo "Content-Type: text/plain"
echo ""
echo "Hello from Shell CGI!"
echo "REQUEST_METHOD=$REQUEST_METHOD"
echo "QUERY_STRING=$QUERY_STRING"
