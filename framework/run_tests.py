import argparse
import subprocess
import sys
import os
import time
import shutil
import glob

# Constants
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD_DIR = os.path.join(PROJECT_ROOT, "build")
WEBSERVER_BIN = os.path.join(BUILD_DIR, "webserv")
UNIT_TEST_SRC = os.path.join(PROJECT_ROOT, "framework", "tests", "unit_test_parser.cpp")
UNIT_TEST_BIN = os.path.join(BUILD_DIR, "unit_test_parser")
COVERAGE_DIR = os.path.join(PROJECT_ROOT, "framework", "coverage")
LCOV_OUTPUT = os.path.join(COVERAGE_DIR, "coverage.info")

def run_command(cmd, cwd=None, env=None, check=True):
    """Run a shell command."""
    print(f"[CMD] {cmd}")
    try:
        subprocess.run(cmd, shell=True, check=check, cwd=cwd, env=env)
        return True
    except subprocess.CalledProcessError:
        print(f"[ERROR] Command failed: {cmd}")
        return False

def clean_project():
    """Clean the project."""
    print("Cleaning project...")
    run_command("make fclean", cwd=PROJECT_ROOT)
    if os.path.exists(COVERAGE_DIR):
        shutil.rmtree(COVERAGE_DIR)
    os.makedirs(COVERAGE_DIR, exist_ok=True)

def build_project(coverage=True):
    """Build the project with optional coverage flags."""
    print("Building project...")
    
    # We use CXX override to inject coverage flags without breaking include paths
    # defined in individual Makefiles.
    cxx_cmd = "c++"
    if coverage:
        print("Enabling coverage flags...")
        cxx_cmd += " --coverage -g -O0"
    
    cmd = f"make re CXX='{cxx_cmd}'"
    return run_command(cmd, cwd=PROJECT_ROOT)

def build_and_run_unit_tests(coverage=True):
    """Compile and run C++ unit tests."""
    print("\n=== Running C++ Unit Tests ===")
    
    # Source files needed for linking. 
    # We link against the static libraries built by the main project.
    
    include_flags = "-Iinclude -Idependencies/libhttp/include -Idependencies/libutils/include -Idependencies/libdir/include"
    # Note: We need to link with gcov if coverage is on.
    
    lib_flags = f"-L{BUILD_DIR}/libhttp -lhttp -L{BUILD_DIR}/libutils -lutils -L{BUILD_DIR}/libdir -ldir"
    
    cxx_flags = "-Wall -Wextra -std=c++98 -g"
    if coverage:
        cxx_flags += " --coverage"
        # --coverage handles -fprofile-arcs -ftest-coverage and linking -lgcov

    # Compile command
    # We need to compile the unit test and link it.
    cmd = f"c++ {cxx_flags} {include_flags} {UNIT_TEST_SRC} -o {UNIT_TEST_BIN} {lib_flags}"
    
    if not run_command(cmd, cwd=PROJECT_ROOT):
        return False
        
    # Run
    if not run_command(UNIT_TEST_BIN, cwd=PROJECT_ROOT):
        return False
        
    print("C++ Unit Tests Passed!")
    return True

CONFIG_FILE = os.path.join(PROJECT_ROOT, "framework", "generated.conf")

def generate_config():
    """Generate a test configuration file with absolute paths."""
    print("Generating test configuration...")
    
    upload_dir = os.path.join(PROJECT_ROOT, "www", "html", "uploads")
    if not os.path.exists(upload_dir):
        os.makedirs(upload_dir)
        
    # We use 'tests' as root for cgi-bin so /cgi-bin/hello.py maps to tests/cgi-bin/hello.py
    # Assuming standard nginx behavior: root + uri
    
    config_content = f"""
http {{
    server {{
        listen 8080;
        server_name localhost;
        root {os.path.join(PROJECT_ROOT, "www", "html")};
        index index.html;
        autoindex on;

        error_page 404 /404.html;

        location / {{
            allowed_methods GET;
            autoindex on;
        }}

        location /uploads {{
            allowed_methods GET POST DELETE PUT;
            upload_enable on;
            upload_store {upload_dir};
            client_max_body_size 100m;
            autoindex on;
        }}

        location /cgi-bin {{
            root {os.path.join(PROJECT_ROOT, "tests")};
            cgi_pass /usr/bin/python3;
            cgi_extension .py;
            allowed_methods GET POST;
        }}
    }}
}}
"""
    with open(CONFIG_FILE, "w") as f:
        f.write(config_content)
    return CONFIG_FILE

def run_integration_tests():
    """Run Python integration tests using unittest."""
    print("\n=== Running Integration Tests ===")
    
    # Generate Config
    config_path = generate_config()
    
    # 1. Start Server
    print(f"Starting Webserver with {config_path}...")
    server_process = None
    log_file = None
    try:
        log_file = open(os.path.join(PROJECT_ROOT, "server_log.txt"), "w")
        server_process = subprocess.Popen(
            [WEBSERVER_BIN, config_path],
            stdout=log_file,
            stderr=subprocess.STDOUT,
            cwd=PROJECT_ROOT
        )
        
        # Wait for server startup
        time.sleep(2) # Give it a moment
        
        # We assume server is on localhost:8080 as per simple.conf
        # Simple check
        import socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        result = sock.connect_ex(('localhost', 8080))
        if result != 0:
            print("[ERROR] Server failed to bind to port 8080")
            server_process.kill()
            return False
        sock.close()

        # 2. Run Tests
        test_dir = os.path.join(PROJECT_ROOT, "framework", "tests")
        env = os.environ.copy()
        
        cmd = f"python3 -m unittest discover -s {test_dir} -p 'test_integration.py'"
        result = run_command(cmd, cwd=PROJECT_ROOT, env=env)
        
        return result

    except Exception as e:
        print(f"[ERROR] Integration tests failed: {e}")
        return False
    finally:
        # 3. Stop Server
        print("Stopping Webserver...")
        if server_process:
            server_process.terminate()
            try:
                server_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                server_process.kill()
        if log_file:
            log_file.close()

def generate_coverage_report():
    """Generate HTML coverage report."""
    print("\n=== Generating Coverage Report ===")
    
    # Check for tools
    if shutil.which("lcov") is None:
        print("[WARN] lcov not found. Skipping coverage report.")
        return
        
    if not os.path.exists(COVERAGE_DIR):
        os.makedirs(COVERAGE_DIR)

    # 1. Capture coverage data
    # Note: We capture from the build directory where .gcda files are
    cmd_capture = f"lcov --capture --directory {BUILD_DIR} --output-file {LCOV_OUTPUT} --base-directory {PROJECT_ROOT} --no-external"
    if not run_command(cmd_capture, cwd=PROJECT_ROOT):
        return

    # 2. Generate HTML
    html_output = os.path.join(COVERAGE_DIR, "html")
    cmd_genhtml = f"genhtml {LCOV_OUTPUT} --output-directory {html_output}"
    run_command(cmd_genhtml, cwd=PROJECT_ROOT)
    
    print(f"Coverage report available at: {html_output}/index.html")

def main():
    parser = argparse.ArgumentParser(description="Run tests for C++ Webserver")
    parser.add_argument("--no-build", action="store_true", help="Skip building")
    parser.add_argument("--no-coverage", action="store_true", help="Disable coverage")
    parser.add_argument("--only-unit", action="store_true", help="Run only unit tests")
    parser.add_argument("--only-integration", action="store_true", help="Run only integration tests")
    
    args = parser.parse_args()
    
    if not args.no_build:
        clean_project()
        if not build_project(coverage=not args.no_coverage):
            print("Build failed.")
            sys.exit(1)

    unit_success = True
    if not args.only_integration:
        unit_success = build_and_run_unit_tests(coverage=not args.no_coverage)

    integration_success = True
    if not args.only_unit:
        integration_success = run_integration_tests()

    if not args.no_coverage and not args.only_unit and not args.only_integration:
        generate_coverage_report()
    elif not args.no_coverage and (args.only_unit or args.only_integration):
        print("Note: Partial coverage generated (subset of tests run).")
        generate_coverage_report()

    if not unit_success or not integration_success:
        sys.exit(1)

if __name__ == "__main__":
    main()
