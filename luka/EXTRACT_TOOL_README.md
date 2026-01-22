# Test Extraction Tool

The `extract_test.py` tool extracts test resources from luka tests into a standalone directory for manual testing without running webserv automatically.

## Usage

```bash
python luka/extract_test.py test_file_name
```

### Examples

```bash
# Extract basic request test
python luka/extract_test.py test_basic_request

# Extract POST test
python luka/extract_test.py test_post

# Extract CGI test
python luka/extract_test.py test_cgi

# Test name can be provided with or without "test_" prefix and ".py" extension
python luka/extract_test.py basic_request  # Also works
```

## What Gets Extracted

The tool creates a directory named after the test (e.g., `basic_request/`) containing:

```
test_name/
├── test_name.conf          # Server configuration file
├── www/                    # Web root with HTML/static files
│   ├── index.html
│   └── ...
├── cgi-bin/                # CGI scripts (if any)
│   ├── script.py
│   └── ...
├── uploads/                # Upload directory (empty)
├── .runme.curl             # Executable bash script with curl commands
└── .runme.telnet           # Text file with telnet commands
```

## Generated Command Files

### `.runme.curl` - Executable Bash Script

An executable script containing all HTTP requests as curl commands:

```bash
#!/bin/bash
# Test: basic_request
# Description: Tests if the server responds to a basic GET request.
# Port: 8080

echo "=== Test: basic_request ==="
echo "Start webserv with: ./build/webserv basic_request/basic_request.conf"
echo ""

# Request 1: GET /
echo ">>> Request 1: GET /"
curl -i -X GET http://localhost:8080/ --http1.0 \
  -H "Connection: close"
echo ""
echo "---"
echo ""
```

**Usage:**
```bash
chmod +x test_name/.runme.curl  # Already executable
./test_name/.runme.curl
```

### `.runme.telnet` - Telnet Commands

A text file with formatted HTTP requests for manual testing with telnet:

```
# Test: basic_request
# Description: Tests if the server responds to a basic GET request.
# Port: 8080
#
# Usage: telnet localhost 8080
# Then copy/paste each request below (including the blank line at the end)

# ========================================
# Request 1: GET /
# ========================================
GET / HTTP/1.0
Host: localhost:8080
Connection: close

```

**Usage:**
```bash
telnet localhost 8080
# Then copy/paste the request from the file
```

## Manual Testing Workflow

### Step 1: Extract Test
```bash
python luka/extract_test.py test_basic_request
```

### Step 2: Start Webserv
```bash
./build/webserv basic_request/basic_request.conf
```

### Step 3: Run Tests

**Option A: Using curl (recommended)**
```bash
./basic_request/.runme.curl
```

**Option B: Using telnet**
```bash
telnet localhost 8080
# Copy/paste requests from basic_request/.runme.telnet
```

**Option C: Manual curl commands**
```bash
curl -i http://localhost:8080/
```

## Features

- **Automatic file extraction**: Copies all test resources (HTML, CSS, JS, CGI scripts, etc.)
- **Path resolution**: Converts paths in configuration to absolute paths
- **Variable resolution**: Extracts literal values from test code variables
- **HTTP request parsing**: Automatically extracts all HTTP requests (GET, POST, PUT, DELETE)
- **Dual command format**: Generates both curl and telnet command files
- **Query string support**: Preserves query parameters in URLs
- **Header support**: Includes custom headers in generated commands
- **POST data support**: Includes request bodies for POST/PUT requests

## Supported HTTP Methods

- GET
- POST (with data and headers)
- PUT (with data and headers)
- DELETE

## Edge Cases Handled

1. **Dynamic paths**: Variables that can't be resolved are shown as placeholders (e.g., `{variable_name}`)
2. **Complex data**: Non-literal data is marked with placeholders
3. **Multiple tests**: If multiple test classes exist in a file, the first one is used
4. **Missing config**: Warns if test doesn't create configuration

## Notes

- The extracted configuration uses absolute paths pointing to the extracted files
- CGI scripts maintain their executable permissions
- The tool does NOT start webserv automatically - you must start it manually
- All paths in the config are absolute, so you can run webserv from any directory
- HTTP/1.0 is used by default (as specified in tests) but can be overridden in the test

## Troubleshooting

### "Error: Test file not found"
- Make sure the test file exists in `luka/tests/`
- Try adding or removing the `test_` prefix

### "Error: Test setup did not create a configuration file"
- The test's `setup()` method must call `create_conf()`
- Check the test file for errors

### "Warning: Could not parse HTTP requests"
- This usually means the test's `run()` method has syntax errors
- The extraction will still work, but command files will be empty

### CGI scripts not executing
- Make sure scripts are executable: `chmod +x cgi_basic/cgi-bin/*.py`
- Check shebang line: `#!/usr/bin/env python3`
- Verify webserv supports CGI and paths are correct

## Advanced Usage

### Testing with different ports

Edit the extracted config file and change the `listen` directive:
```nginx
listen 9090;  # Changed from 8080
```

Then update the port in curl/telnet commands accordingly.

### Adding custom requests

You can edit `.runme.curl` to add your own curl commands:
```bash
# Custom test
echo ">>> Custom: Testing 404"
curl -i -X GET http://localhost:8080/nonexistent
echo ""
```

### Testing with different HTTP versions

For HTTP/1.1, remove the `--http1.0` flag from curl commands:
```bash
curl -i -X GET http://localhost:8080/
```

## Examples

See the following extracted tests for reference:
- `basic_request/` - Simple GET request
- `post_request/` - POST with form data and CGI
- `cgi_basic/` - CGI scripts with query strings

## Integration with Development

This tool is useful for:
- Debugging specific test scenarios manually
- Creating minimal reproducible examples for bug reports
- Testing server behavior outside the test framework
- Sharing test cases with team members
- Validating server responses manually
