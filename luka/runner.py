import os
import sys
import importlib.util
import time
import re
import tempfile
import argparse

# Add project root to sys.path to allow for absolute imports
WEBSERV_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, WEBSERV_PATH)

from luka import logger, assertions
from luka.utilities import process, network, http, concurrent
from luka.base_test import BaseTest

TESTS_DIR = os.path.join(WEBSERV_PATH, "luka", "tests")
TEST_RUNS_DIR = os.path.join(WEBSERV_PATH, "luka", "test_runs")
WEBSERV_BIN = os.path.join(WEBSERV_PATH, "build/webserv")
SERVER_START_TIMEOUT = 5  # seconds

# Global flag for valgrind mode
USE_VALGRIND = False


def compile_webserv():
    logger.info("Compiling webserv...")
    return_code, _, stderr = process.run_command(["make"], cwd=WEBSERV_PATH)
    if return_code != 0:
        logger.error("Failed to compile webserv.")
        logger.error(stderr)
        sys.exit(1)
    logger.passed("Compilation successful.")
    print("")
    print("===== Starting Tests =====")
    print("")


def find_tests():
    """Find all test classes that inherit from BaseTest (searches recursively in subdirectories)."""
    test_classes = []

    # Classes to exclude (base classes, not actual tests)
    excluded_classes = {"BaseTest", "RawRequestTest", "ConfigTestBase"}

    # Walk through TESTS_DIR and subdirectories
    for root, dirs, files in os.walk(TESTS_DIR):
        # Skip __pycache__ directories
        dirs[:] = [d for d in dirs if d != "__pycache__"]

        for f in files:
            if f.startswith("test_") and f.endswith(".py"):
                test_path = os.path.join(root, f)
                spec = importlib.util.spec_from_file_location("test_module", test_path)
                test_module = importlib.util.module_from_spec(spec)
                try:
                    spec.loader.exec_module(test_module)

                    # Find all BaseTest subclasses in the module
                    for item_name in dir(test_module):
                        item = getattr(test_module, item_name)
                        if (
                            isinstance(item, type)
                            and issubclass(item, BaseTest)
                            and item is not BaseTest
                            and item.__name__ not in excluded_classes
                        ):
                            test_classes.append(item)
                except Exception as e:
                    logger.error(f"Failed to load test module {f}: {e}")

    return test_classes


def parse_valgrind_output(valgrind_log, test_name):
    """Parse valgrind output and highlight memory leaks in orange."""
    try:
        with open(valgrind_log, "r") as f:
            content = f.read()

        # Check for leak summary
        leak_summary_match = re.search(
            r"LEAK SUMMARY:.*?(?=\n\n|\Z)", content, re.DOTALL
        )
        heap_summary_match = re.search(
            r"HEAP SUMMARY:.*?(?=\n\n|\Z)", content, re.DOTALL
        )

        # Check for "no leaks are possible"
        if "All heap blocks were freed -- no leaks are possible" in content:
            logger.passed(f"Valgrind: No memory leaks detected in '{test_name}'")
            return

        # If there are leaks, print them in orange
        has_leaks = False
        if leak_summary_match:
            leak_text = leak_summary_match.group(0)
            # Check if there are actual leaks (not all 0 bytes)
            # Note: valgrind uses commas in numbers (e.g., "9,648 bytes")
            if (
                re.search(r"definitely lost: [1-9][\d,]* bytes", leak_text)
                or re.search(r"indirectly lost: [1-9][\d,]* bytes", leak_text)
                or re.search(r"possibly lost: [1-9][\d,]* bytes", leak_text)
            ):
                has_leaks = True

        if has_leaks:
            logger.orange(f"\n{'=' * 60}")
            logger.orange(f"MEMORY LEAKS DETECTED IN TEST: {test_name}")
            logger.orange(f"{'=' * 60}")

            if heap_summary_match:
                logger.orange("\n" + heap_summary_match.group(0))

            if leak_summary_match:
                logger.orange("\n" + leak_summary_match.group(0))

            # Print error records with leak details
            error_records = re.findall(
                r"==\d+== \d+ bytes in \d+ blocks are (?:definitely|indirectly|possibly) lost.*?(?=\n==\d+==\s*$|\Z)",
                content,
                re.DOTALL | re.MULTILINE,
            )
            if error_records:
                logger.orange("\nLeak Details:")
                for record in error_records[:5]:  # Limit to first 5 records
                    logger.orange(record.strip())

            logger.orange(f"\n{'=' * 60}")
            logger.orange(f"Full valgrind log: {valgrind_log}")
            logger.orange(f"{'=' * 60}\n")
        else:
            logger.passed(f"Valgrind: No memory leaks in '{test_name}'")

    except Exception as e:
        logger.error(f"Failed to parse valgrind output: {e}")


def run_test(test_class):
    """Run a single test class."""
    test_instance = test_class()
    test_name = test_instance.get_name()
    test_description = test_instance.get_description()

    logger.info(f"Running test: {test_name}")
    logger.info(f"Description: {test_description}")

    # Initialize test environment
    test_instance.initialize_test_env(TEST_RUNS_DIR)

    webserv_process = None
    try:
        # Inject HTTP and assertion methods
        def get_wrapper(path, headers=None, http_version="1.0", timeout=5):
            url = test_instance.get_url(path)
            response = http.get(
                url, headers=headers, http_version=http_version, timeout=timeout
            )
            if response is None:
                raise AssertionError(
                    f"GET request to {url} failed - no response received"
                )
            return response

        def head_wrapper(path, headers=None, http_version="1.0", timeout=5):
            url = test_instance.get_url(path)
            response = http.head(
                url, headers=headers, http_version=http_version, timeout=timeout
            )
            if response is None:
                raise AssertionError(
                    f"HEAD request to {url} failed - no response received"
                )
            return response

        def post_wrapper(path, data=None, json=None, headers=None, http_version="1.0"):
            url = test_instance.get_url(path)
            response = http.post(
                url, data=data, json=json, headers=headers, http_version=http_version
            )
            if response is None:
                raise AssertionError(
                    f"POST request to {url} failed - no response received"
                )
            return response

        def delete_wrapper(path, headers=None, http_version="1.0"):
            url = test_instance.get_url(path)
            response = http.delete(url, headers=headers, http_version=http_version)
            if response is None:
                raise AssertionError(
                    f"DELETE request to {url} failed - no response received"
                )
            return response

        def put_wrapper(path, data=None, json=None, headers=None, http_version="1.0"):
            url = test_instance.get_url(path)
            response = http.put(
                url, data=data, json=json, headers=headers, http_version=http_version
            )
            if response is None:
                raise AssertionError(
                    f"PUT request to {url} failed - no response received"
                )
            return response

        def assert_equal_wrapper(expected, actual, message=""):
            assertions.assertEqual(expected, actual, message)

        def assert_contains_wrapper(text, substring, message=""):
            if substring not in text:
                raise assertions.AssertEqualError(
                    f"Text containing '{substring}'", text
                )

        test_instance.get = get_wrapper
        test_instance.head = head_wrapper
        test_instance.post = post_wrapper
        test_instance.delete = delete_wrapper
        test_instance.put = put_wrapper
        test_instance.assert_equal = assert_equal_wrapper
        test_instance.assert_contains = assert_contains_wrapper

        # Inject concurrent request helpers
        test_instance.concurrent_get = concurrent.make_concurrent_get_helper(http)
        test_instance.concurrent_requests = concurrent.make_concurrent_requests_helper(
            http
        )

        # Run test setup
        test_instance.setup()

        if not test_instance.conf_path or not os.path.exists(test_instance.conf_path):
            logger.error("Test setup did not create a configuration file.")
            return False

        # Extract port from config
        with open(test_instance.conf_path, "r") as f:
            conf_content = f.read()

        port_match = re.search(r"listen\s+(\d+);", conf_content)
        if not port_match:
            logger.error("Could not find port in test config.")
            return False
        port = int(port_match.group(1))

        # Start webserv
        if USE_VALGRIND:
            command = [
                "valgrind",
                "--leak-check=full",
                "--show-leak-kinds=all",
                "--track-origins=yes",
                "--log-file=" + os.path.join(test_instance.logs_dir, "valgrind.log"),
                WEBSERV_BIN,
                test_instance.conf_path,
            ]
            logger.info(f"Starting webserv with valgrind")
        else:
            command = [WEBSERV_BIN, test_instance.conf_path]
        # logger.info(f"Starting webserv: {' '.join(command)}")

        webserv_process = process.run_command(command, wait=False)

        # Wait for server to be ready
        start_time = time.time()
        while not network.is_port_open("localhost", port):
            if webserv_process.poll() is not None:
                logger.error(
                    f"Server process terminated unexpectedly with code {webserv_process.returncode}."
                )
                try:
                    stdout, stderr = webserv_process.communicate(timeout=0.1)
                    if stdout:
                        logger.error(f"Server stdout:\n{stdout}")
                    if stderr:
                        logger.error(f"Server stderr:\n{stderr}")
                except:
                    pass
                return False
            if time.time() - start_time > SERVER_START_TIMEOUT:
                logger.error(
                    f"Server did not start on port {port} within {SERVER_START_TIMEOUT} seconds."
                )
                webserv_process.kill()
                try:
                    stdout, stderr = webserv_process.communicate(timeout=1)
                    if stdout:
                        logger.error(f"Server stdout:\n{stdout}")
                    if stderr:
                        logger.error(f"Server stderr:\n{stderr}")
                except:
                    pass
                return False
            time.sleep(0.1)

        # logger.info(f"Server started on port {port}.")

        # Run test
        test_instance.run()
        logger.passed(f"Test '{test_name}' passed.")
        return True

    except assertions.AssertEqualError as e:
        logger.failed(f"Test '{test_name}' failed: {e}")
        return False
    except Exception as e:
        logger.error(f"An error occurred during test '{test_name}': {e}")
        import traceback

        logger.error(traceback.format_exc())
        return False
    finally:
        if webserv_process:
            # Use SIGINT for clean shutdown (allows valgrind to write leak summary)
            # SIGKILL prevents valgrind from finalizing its report
            try:
                webserv_process.terminate()  # Sends SIGTERM (clean shutdown)
                webserv_process.wait(timeout=2)
            except:
                # If SIGTERM didn't work, force kill
                webserv_process.kill()
                try:
                    webserv_process.wait(timeout=1)
                except:
                    pass

        # Parse and display valgrind output if in valgrind mode
        if USE_VALGRIND:
            # Give valgrind time to finalize the log
            time.sleep(0.5)

            valgrind_log = os.path.join(test_instance.logs_dir, "valgrind.log")
            if os.path.exists(valgrind_log):
                parse_valgrind_output(valgrind_log, test_name)

        # Print newlines after test (and valgrind output if present)
        print("\n\n", file=sys.stderr)

        # Cleanup test environment
        test_instance.cleanup_test_env()


def main():
    """Main entry point for the test runner."""
    global USE_VALGRIND

    # Parse command-line arguments
    parser = argparse.ArgumentParser(
        description="Luka Testing Framework for webserv",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 runner.py              # Run all tests normally
  python3 runner.py -v           # Run all tests with valgrind
  python3 runner.py basic        # Run specific test
  python3 runner.py -v basic     # Run specific test with valgrind
        """,
    )
    parser.add_argument(
        "-v",
        "--valgrind",
        action="store_true",
        help="Run server with valgrind for memory leak detection",
    )
    parser.add_argument(
        "test_name", nargs="?", help="Specific test name to run (optional)"
    )

    args = parser.parse_args()
    USE_VALGRIND = args.valgrind

    if USE_VALGRIND:
        logger.info("Valgrind mode enabled - tests will run slower")
        # Check if valgrind is installed
        try:
            import subprocess

            result = subprocess.run(
                ["which", "valgrind"], capture_output=True, text=True
            )
            if result.returncode != 0:
                logger.error("Valgrind is not installed. Please install it first.")
                logger.error("On Ubuntu/Debian: sudo apt-get install valgrind")
                logger.error("On macOS: brew install valgrind")
                sys.exit(1)
        except Exception as e:
            logger.error(f"Failed to check for valgrind: {e}")
            sys.exit(1)

    if not os.path.exists(TEST_RUNS_DIR):
        os.makedirs(TEST_RUNS_DIR)

    compile_webserv()

    test_classes = find_tests()
    if not test_classes:
        logger.warn("No tests found.")
        return

    # Check if a specific test name was provided
    target_test = args.test_name
    if target_test:
        logger.info(f"Running specific test: {target_test}")

        # Filter test classes to match the target
        filtered_classes = []
        for test_class in test_classes:
            test_instance = test_class()
            test_name = test_instance.get_name()
            class_name = test_class.__name__

            # Match against class name or test name
            if (
                target_test.lower() in test_name.lower()
                or target_test.lower() in class_name.lower()
            ):
                filtered_classes.append(test_class)

        if not filtered_classes:
            logger.error(f"No test found matching '{target_test}'")
            return

        test_classes = filtered_classes

    results = {"passed": 0, "failed": 0}
    for test_class in test_classes:
        if run_test(test_class):
            results["passed"] += 1
        else:
            results["failed"] += 1

    logger.info("-" * 20)
    logger.info("Test Results:")
    logger.passed(f"Passed: {results['passed']}")
    logger.failed(f"Failed: {results['failed']}")
    logger.info("-" * 20)


if __name__ == "__main__":
    main()
