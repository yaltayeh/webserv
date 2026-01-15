# Webserver Testing Framework

This is a comprehensive Python-based testing framework for the C++ Webserver project. It includes integration testing, C++ unit testing, and code coverage analysis.

## Requirements

*   Python 3.x
*   `unittest` (Standard Library)
*   `requests` (`pip install requests`)
*   `lcov` (For coverage reports, e.g., `apt install lcov` or `brew install lcov`)

## Directory Structure

*   `framework/run_tests.py`: Main entry point. Handles building, running tests, and generating reports.
*   `framework/tests/`: Contains Python integration tests and C++ unit tests.
*   `framework/tests/test_integration.py`: Integration test suite (GET, POST, DELETE, Chunked, CGI).
*   `framework/tests/unit_test_parser.cpp`: Unit tests for internal C++ logic (Parser).
*   `framework/coverage/`: Generated coverage reports.
*   `framework/generated.conf`: Auto-generated configuration file for the test server.

## Usage

Run the full suite:

```bash
python3 framework/run_tests.py
```

This will:
1.  Clean and Rebuild the project with coverage flags (`--coverage`).
2.  Compile and run C++ unit tests.
3.  Start the webserver with a custom test configuration.
4.  Run Python integration tests.
5.  Generate an HTML coverage report in `framework/coverage/html/index.html`.

### Options

*   `--no-build`: Skip the build step (useful for rapid test iteration).
*   `--no-coverage`: Disable coverage flags (faster build).
*   `--only-unit`: Run only C++ unit tests.
*   `--only-integration`: Run only Python integration tests.

## Troubleshooting Results

*   **404 on Uploads:** If `POST /uploads/...` returns 404, check if your server handles file creation correctly for POST requests. Ensure it doesn't return 404 just because the file doesn't exist *yet*.
*   **500 on CGI:** Check `server_log.txt`. Often caused by pipe handling issues or script permission errors.
*   **Timeouts:** If tests hang, the server might be waiting for more data or failing to send a response. Check `Content-Length` handling.

## Coverage Report

Open `framework/coverage/html/index.html` in your browser to inspect line-by-line coverage of your C++ code.
