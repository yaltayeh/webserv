#!/bin/bash

# Test HEAD request implementation
# This script tests that HEAD requests work correctly

PORT=8080
HOST="localhost"
FILE="/index.html"

echo "Testing HEAD request implementation..."
echo "========================================"
echo ""

# Start the server in the background
echo "Starting server..."
PORT=$PORT ./build/webserv conf/simple.conf &
SERVER_PID=$!

# Wait for server to start
sleep 2

# Test 1: HEAD request
echo "Test 1: HEAD request should return headers without body"
echo "--------------------------------------------------------"
RESPONSE=$(printf "HEAD $FILE HTTP/1.0\r\nHost: $HOST\r\n\r\n" | nc $HOST $PORT)
echo "$RESPONSE"
echo ""

# Count lines in response (should be just headers, no body)
LINES=$(echo "$RESPONSE" | wc -l)
echo "Number of lines in HEAD response: $LINES"
echo ""

# Test 2: GET request for comparison
echo "Test 2: GET request should return headers AND body"
echo "---------------------------------------------------"
RESPONSE_GET=$(printf "GET $FILE HTTP/1.0\r\nHost: $HOST\r\n\r\n" | nc $HOST $PORT)
echo "$RESPONSE_GET"
echo ""

# Count lines in GET response
LINES_GET=$(echo "$RESPONSE_GET" | wc -l)
echo "Number of lines in GET response: $LINES_GET"
echo ""

# Verify HEAD has fewer lines than GET
if [ $LINES -lt $LINES_GET ]; then
    echo "✓ PASS: HEAD response has fewer lines than GET (no body)"
else
    echo "✗ FAIL: HEAD response should have fewer lines than GET"
fi
echo ""

# Stop the server
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "Test complete!"
