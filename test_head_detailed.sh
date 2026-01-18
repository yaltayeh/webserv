#!/bin/bash

# More detailed HEAD request test
PORT=8080
HOST="localhost"

echo "Detailed HEAD Request Test"
echo "==========================="
echo ""

# Start the server
PORT=$PORT ./build/webserv conf/simple.conf &
SERVER_PID=$!
sleep 2

echo "1. Testing HEAD request on /index.html"
echo "---------------------------------------"
HEAD_RESPONSE=$(printf "HEAD /index.html HTTP/1.0\r\nHost: $HOST\r\n\r\n" | nc $HOST $PORT)
echo "Response:"
echo "$HEAD_RESPONSE"
echo ""
HEAD_SIZE=$(echo -n "$HEAD_RESPONSE" | wc -c)
echo "HEAD response size: $HEAD_SIZE bytes"
echo ""

echo "2. Testing GET request on /index.html"
echo "--------------------------------------"
GET_RESPONSE=$(printf "GET /index.html HTTP/1.0\r\nHost: $HOST\r\n\r\n" | nc $HOST $PORT)
echo "Response:"
echo "$GET_RESPONSE"
echo ""
GET_SIZE=$(echo -n "$GET_RESPONSE" | wc -c)
echo "GET response size: $GET_SIZE bytes"
echo ""

echo "3. Comparison"
echo "-------------"
echo "HEAD size: $HEAD_SIZE bytes"
echo "GET size:  $GET_SIZE bytes"
echo "Difference: $((GET_SIZE - HEAD_SIZE)) bytes"
echo ""

# Check if HEAD has same headers but no body
if echo "$HEAD_RESPONSE" | grep -q "Content-Length: 59"; then
    echo "✓ PASS: HEAD response includes Content-Length header"
else
    echo "✗ FAIL: HEAD response missing Content-Length header"
fi

if echo "$HEAD_RESPONSE" | grep -q "Content-Type: text/html"; then
    echo "✓ PASS: HEAD response includes Content-Type header"
else
    echo "✗ FAIL: HEAD response missing Content-Type header"
fi

if ! echo "$HEAD_RESPONSE" | grep -q "<h1>"; then
    echo "✓ PASS: HEAD response does not include body content"
else
    echo "✗ FAIL: HEAD response incorrectly includes body content"
fi

if [ $HEAD_SIZE -lt $GET_SIZE ]; then
    echo "✓ PASS: HEAD response is smaller than GET response"
else
    echo "✗ FAIL: HEAD response should be smaller than GET"
fi

# Kill server
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "Test complete!"
