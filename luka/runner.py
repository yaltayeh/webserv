import os
import sys
import importlib.util
import time
import re
import tempfile

# Add project root to sys.path to allow for absolute imports
WEBSERV_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, WEBSERV_PATH)

from luka import logger, assertions
from luka.utilities import process, network, http
from luka.base_test import BaseTest

TESTS_DIR = os.path.join(WEBSERV_PATH, "loka", "tests")
TEST_RUNS_DIR = os.path.join(WEBSERV_PATH, "loka", "test_runs")
WEBSERV_BIN = os.path.join(WEBSERV_PATH, "build/webserv")
SERVER_START_TIMEOUT = 5  # seconds

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
    """Find all test classes that inherit from BaseTest."""
    test_classes = []
    for f in os.listdir(TESTS_DIR):
        if f.startswith("test_") and f.endswith(".py"):
            test_path = os.path.join(TESTS_DIR, f)
            spec = importlib.util.spec_from_file_location("test_module", test_path)
            test_module = importlib.util.module_from_spec(spec)
            try:
                spec.loader.exec_module(test_module)
                
                # Find all BaseTest subclasses in the module
                for item_name in dir(test_module):
                    item = getattr(test_module, item_name)
                    if (isinstance(item, type) and 
                        issubclass(item, BaseTest) and 
                        item is not BaseTest):
                        test_classes.append(item)
            except Exception as e:
                logger.error(f"Failed to load test module {f}: {e}")
    
    return test_classes

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
        def get_wrapper(path, headers=None, http_version="1.0"):
            url = test_instance.get_url(path)
            response = http.get(url, headers=headers, http_version=http_version)
            if response is None:
                raise AssertionError(f"GET request to {url} failed - no response received")
            return response
        
        def post_wrapper(path, data=None, json=None, headers=None, http_version="1.0"):
            url = test_instance.get_url(path)
            response = http.post(url, data=data, json=json, headers=headers, http_version=http_version)
            if response is None:
                raise AssertionError(f"POST request to {url} failed - no response received")
            return response
        
        def assert_equal_wrapper(expected, actual, message=""):
            assertions.assertEqual(expected, actual, message)
        
        def assert_contains_wrapper(text, substring, message=""):
            if substring not in text:
                raise assertions.AssertEqualError(f"Text containing '{substring}'", text)
        
        test_instance.get = get_wrapper
        test_instance.post = post_wrapper
        test_instance.assert_equal = assert_equal_wrapper
        test_instance.assert_contains = assert_contains_wrapper
        
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
        command = [WEBSERV_BIN, test_instance.conf_path]
        # logger.info(f"Starting webserv: {' '.join(command)}")
        
        webserv_process = process.run_command(command, wait=False)
        
        # Wait for server to be ready
        start_time = time.time()
        while not network.is_port_open("localhost", port):
            if webserv_process.poll() is not None:
                logger.error(f"Server process terminated unexpectedly with code {webserv_process.returncode}.")
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
                logger.error(f"Server did not start on port {port} within {SERVER_START_TIMEOUT} seconds.")
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
        logger.passed(f"Test '{test_name}' passed.\n\n")
        return True
        
    except assertions.AssertEqualError as e:
        logger.failed(f"Test '{test_name}' failed: {e} \n\n")
        return False
    except Exception as e:
        logger.error(f"An error occurred during test '{test_name}': {e}")
        import traceback
        logger.error(traceback.format_exc() + "\n\n")
        return False
    finally:
        if webserv_process:
            webserv_process.kill()
            try:
                webserv_process.wait(timeout=1)
            except:
                pass
        
        # Cleanup test environment
        test_instance.cleanup_test_env()


def main():
    """Main entry point for the test runner."""
    if not os.path.exists(TEST_RUNS_DIR):
        os.makedirs(TEST_RUNS_DIR)

    compile_webserv()
    
    test_classes = find_tests()
    if not test_classes:
        logger.warn("No tests found.")
        return

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
