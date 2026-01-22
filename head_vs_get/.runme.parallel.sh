#!/bin/bash
# Test: head_vs_get
# Description: Tests HEAD method returns same headers as GET but without body.
# Port: 8080
#
# This script executes requests in parallel where appropriate

echo "=== Test: head_vs_get (Parallel Execution) ==="
echo "Start webserv with: ./build/webserv head_vs_get/head_vs_get.conf"
echo ""

# Sequential request 1: HEAD /
echo ">>> Request 1: HEAD /"
curl -i -X HEAD http://localhost:8080/ --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 2: GET /
echo ">>> Request 2: GET /"
curl -i -X GET http://localhost:8080/ --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 3: HEAD /large.html
echo ">>> Request 3: HEAD /large.html"
curl -i -X HEAD http://localhost:8080/large.html --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 4: GET /large.html
echo ">>> Request 4: GET /large.html"
curl -i -X GET http://localhost:8080/large.html --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 5: GET Content-Length
echo ">>> Request 5: GET Content-Length"
curl -i -X GET http://localhost:8080Content-Length --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 6: GET Content-Length
echo ">>> Request 6: GET Content-Length"
curl -i -X GET http://localhost:8080Content-Length --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 7: GET Content-Type
echo ">>> Request 7: GET Content-Type"
curl -i -X GET http://localhost:8080Content-Type --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
# Sequential request 8: GET Content-Type
echo ">>> Request 8: GET Content-Type"
curl -i -X GET http://localhost:8080Content-Type --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""