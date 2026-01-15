"""
Base test class for luka testing framework.
Provides isolated test environments with dedicated directories and resources.
"""
import os
import shutil
import tempfile
from abc import ABC, abstractmethod
from typing import Dict, Optional, List


class BaseTest(ABC):
    """
    Base class for all webserv tests.
    
    Each test gets an isolated environment with:
    - conf/: Configuration files
    - www/: Web root directory
    - uploads/: Upload directory
    - cgi-bin/: CGI scripts directory
    - logs/: Log files directory
    
    Usage:
        class MyTest(BaseTest):
            def setup(self):
                self.create_html_file("index.html", "<h1>Hello</h1>")
                self.create_conf('''
                    http {
                        server {
                            listen 8080;
                            root {www_root};
                            index index.html;
                        }
                    }
                ''')
            
            def run(self):
                response = self.get("/")
                self.assert_equal(200, response.status_code)
    """
    
    def __init__(self):
        self.test_dir: Optional[str] = None
        self.conf_dir: Optional[str] = None
        self.www_dir: Optional[str] = None
        self.uploads_dir: Optional[str] = None
        self.cgi_dir: Optional[str] = None
        self.logs_dir: Optional[str] = None
        self.conf_path: Optional[str] = None
        self.port: int = 8080
        self._workspace_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    
    @abstractmethod
    def get_name(self) -> str:
        """Return the test name (used for directory naming)."""
        pass
    
    @abstractmethod
    def get_description(self) -> str:
        """Return the test description."""
        pass
    
    @abstractmethod
    def setup(self):
        """
        Setup test environment.
        Create config files, HTML files, CGI scripts, etc.
        Must call create_conf() to generate the configuration.
        """
        pass
    
    @abstractmethod
    def run(self):
        """
        Run the actual test logic.
        Use self.get(), self.post(), self.assert_equal(), etc.
        """
        pass
    
    def initialize_test_env(self, base_test_dir: str):
        """Initialize isolated test environment directories."""
        self.test_dir = os.path.join(base_test_dir, self.get_name())
        self.conf_dir = os.path.join(self.test_dir, "conf")
        self.www_dir = os.path.join(self.test_dir, "www")
        self.uploads_dir = os.path.join(self.test_dir, "uploads")
        self.cgi_dir = os.path.join(self.test_dir, "cgi-bin")
        self.logs_dir = os.path.join(self.test_dir, "logs")
        
        # Create directories
        for directory in [self.conf_dir, self.www_dir, self.uploads_dir, 
                         self.cgi_dir, self.logs_dir]:
            os.makedirs(directory, exist_ok=True)
    
    def cleanup_test_env(self):
        """Clean up test environment."""
        if self.test_dir and os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir, ignore_errors=True)
    
    def create_conf(self, conf_content: str, replacements: Optional[Dict[str, str]] = None):
        """
        Create configuration file with variable substitution.
        
        Args:
            conf_content: Configuration file content
            replacements: Dictionary of placeholders to replace (e.g., {www_root}, {port})
        """
        # Default replacements
        default_replacements = {
            "{www_root}": self.www_dir,
            "{uploads_dir}": self.uploads_dir,
            "{cgi_dir}": self.cgi_dir,
            "{logs_dir}": self.logs_dir,
            "{port}": str(self.port),
        }
        
        if replacements:
            default_replacements.update(replacements)
        
        # Apply replacements
        for placeholder, value in default_replacements.items():
            conf_content = conf_content.replace(placeholder, value)
        
        self.conf_path = os.path.join(self.conf_dir, f"{self.get_name()}.conf")
        with open(self.conf_path, "w") as f:
            f.write(conf_content)
    
    def create_html_file(self, filename: str, content: str, subdir: str = ""):
        """
        Create an HTML file in the www directory.
        
        Args:
            filename: Name of the HTML file
            content: HTML content
            subdir: Optional subdirectory within www/
        """
        target_dir = os.path.join(self.www_dir, subdir) if subdir else self.www_dir
        os.makedirs(target_dir, exist_ok=True)
        
        filepath = os.path.join(target_dir, filename)
        with open(filepath, "w") as f:
            f.write(content)
    
    def create_cgi_script(self, filename: str, content: str, executable: bool = True):
        """
        Create a CGI script in the cgi-bin directory.
        
        Args:
            filename: Name of the CGI script
            content: Script content
            executable: Make the script executable
        """
        filepath = os.path.join(self.cgi_dir, filename)
        with open(filepath, "w") as f:
            f.write(content)
        
        if executable:
            os.chmod(filepath, 0o755)
    
    def create_file(self, filename: str, content: str, subdir: str = ""):
        """
        Create any file in the www directory.
        
        Args:
            filename: Name of the file
            content: File content
            subdir: Optional subdirectory within www/
        """
        target_dir = os.path.join(self.www_dir, subdir) if subdir else self.www_dir
        os.makedirs(target_dir, exist_ok=True)
        
        filepath = os.path.join(target_dir, filename)
        with open(filepath, "w") as f:
            f.write(content)
    
    def create_directory(self, dirname: str, parent: str = "www"):
        """
        Create a directory in the test environment.
        
        Args:
            dirname: Name of the directory
            parent: Parent directory ('www', 'uploads', 'cgi-bin')
        """
        parent_map = {
            "www": self.www_dir,
            "uploads": self.uploads_dir,
            "cgi-bin": self.cgi_dir,
        }
        
        parent_dir = parent_map.get(parent, self.www_dir)
        dirpath = os.path.join(parent_dir, dirname)
        os.makedirs(dirpath, exist_ok=True)
    
    def set_port(self, port: int):
        """Set the port for this test."""
        self.port = port
    
    def get_url(self, path: str = "/") -> str:
        """Get full URL for a path."""
        return f"http://localhost:{self.port}{path}"
    
    # HTTP helper methods (will be injected by runner)
    def get(self, path: str, headers: Optional[Dict] = None, http_version: str = "1.0"):
        """Make a GET request (implemented by runner)."""
        raise NotImplementedError("HTTP methods are injected by runner")
    
    def post(self, path: str, data=None, json=None, headers: Optional[Dict] = None, 
             http_version: str = "1.0"):
        """Make a POST request (implemented by runner)."""
        raise NotImplementedError("HTTP methods are injected by runner")
    
    def assert_equal(self, expected, actual, message: str = ""):
        """Assert two values are equal (implemented by runner)."""
        raise NotImplementedError("Assertion methods are injected by runner")
    
    def assert_contains(self, text: str, substring: str, message: str = ""):
        """Assert text contains substring (implemented by runner)."""
        raise NotImplementedError("Assertion methods are injected by runner")
    
    def assert_status(self, response, expected_status: int, message: str = ""):
        """Assert response has expected status code."""
        if response is None:
            raise AssertionError("HTTP request failed - response is None. Check server logs.")
        self.assert_equal(expected_status, response.status_code, 
                         message or f"Expected status {expected_status}")
