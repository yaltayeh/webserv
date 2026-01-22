"""
Concurrent request utilities for the luka testing framework.
Enables running multiple HTTP requests in parallel using threading.
"""

import time
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import List, Dict, Optional, Any, Callable, Tuple
from luka import logger


class ConcurrentResult:
    """Wrapper for individual concurrent request results."""

    def __init__(
        self,
        request_id: int,
        response: Any,
        duration: float,
        error: Optional[Exception] = None,
        thread_id: Optional[int] = None,
    ):
        self.request_id = request_id
        self.response = response
        self.duration = duration
        self.error = error
        self.thread_id = thread_id
        self.success = error is None and response is not None

    def __repr__(self):
        status = "SUCCESS" if self.success else "FAILED"
        return f"ConcurrentResult(id={self.request_id}, status={status}, duration={self.duration:.2f}s)"


def run_concurrent_requests(
    request_specs: List[Dict[str, Any]], max_workers: Optional[int] = None
) -> List[ConcurrentResult]:
    """
    Run multiple HTTP requests concurrently using ThreadPoolExecutor.

    Args:
        request_specs: List of request specifications. Each spec is a dict with:
            - 'func': Callable that performs the request
            - 'args': Tuple of positional arguments (optional)
            - 'kwargs': Dict of keyword arguments (optional)
            - 'id': Request identifier (optional, defaults to index)
        max_workers: Maximum number of concurrent threads (default: len(request_specs))

    Returns:
        List of ConcurrentResult objects with timing and error information

    Example:
        request_specs = [
            {'func': lambda: http.get('http://localhost:8080/')},
            {'func': lambda: http.get('http://localhost:8080/page2')},
            {'func': lambda: http.post('http://localhost:8080/api', data='test')},
        ]
        results = run_concurrent_requests(request_specs)
        for result in results:
            if result.success:
                print(f"Request {result.request_id}: {result.response.status_code}")
    """
    if max_workers is None:
        max_workers = len(request_specs)

    results = []

    def execute_request(spec: Dict[str, Any], request_id: int) -> ConcurrentResult:
        """Execute a single request and capture timing/error info."""
        func = spec["func"]
        args = spec.get("args", ())
        kwargs = spec.get("kwargs", {})
        thread_id = threading.get_ident()

        start_time = time.time()
        response = None
        error = None

        try:
            response = func(*args, **kwargs)
        except Exception as e:
            error = e
            logger.error(f"Request {request_id} failed: {e}")

        duration = time.time() - start_time
        return ConcurrentResult(request_id, response, duration, error, thread_id)

    # Execute requests concurrently
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # Submit all tasks
        future_to_id = {}
        for idx, spec in enumerate(request_specs):
            request_id = spec.get("id", idx)
            future = executor.submit(execute_request, spec, request_id)
            future_to_id[future] = request_id

        # Collect results as they complete
        for future in as_completed(future_to_id):
            try:
                result = future.result()
                results.append(result)
            except Exception as e:
                request_id = future_to_id[future]
                logger.error(f"Fatal error executing request {request_id}: {e}")
                results.append(ConcurrentResult(request_id, None, 0.0, e))

    # Sort results by request_id for consistent ordering
    results.sort(key=lambda r: r.request_id)
    return results


def make_concurrent_get_helper(http_module):
    """
    Factory function to create concurrent_get helper bound to http module.
    Used by runner.py to inject the helper into test instances.

    Args:
        http_module: The luka.utilities.http module

    Returns:
        concurrent_get function
    """

    def concurrent_get(
        urls: List[str],
        headers: Optional[Dict] = None,
        http_version: str = "1.0",
        max_workers: Optional[int] = None,
        timeout: int = 5,
    ) -> List[ConcurrentResult]:
        """
        Make multiple concurrent GET requests.

        Args:
            urls: List of URLs to request
            headers: Optional headers to include in all requests
            http_version: HTTP version to use (default: "1.0")
            max_workers: Maximum concurrent threads (default: len(urls))
            timeout: Request timeout in seconds (default: 5)

        Returns:
            List of ConcurrentResult objects
        """
        request_specs = [
            {
                "func": http_module.get,
                "kwargs": {
                    "url": url,
                    "headers": headers,
                    "http_version": http_version,
                    "timeout": timeout,
                },
                "id": idx,
            }
            for idx, url in enumerate(urls)
        ]
        return run_concurrent_requests(request_specs, max_workers)

    return concurrent_get


def make_concurrent_requests_helper(http_module):
    """
    Factory function to create concurrent_requests helper bound to http module.
    Used by runner.py to inject the helper into test instances.

    Args:
        http_module: The luka.utilities.http module

    Returns:
        concurrent_requests function
    """

    def concurrent_requests(
        request_list: List[Dict[str, Any]], max_workers: Optional[int] = None
    ) -> List[ConcurrentResult]:
        """
        Make multiple concurrent HTTP requests with different methods.

        Args:
            request_list: List of request dicts, each containing:
                - 'method': HTTP method ('GET', 'POST', 'PUT', 'DELETE', 'HEAD')
                - 'url': Target URL
                - 'headers': Optional headers dict
                - 'data': Optional request body (for POST/PUT)
                - 'http_version': Optional HTTP version (default: "1.0")
            max_workers: Maximum concurrent threads (default: len(request_list))

        Returns:
            List of ConcurrentResult objects

        Example:
            results = test.concurrent_requests([
                {'method': 'GET', 'url': test.get_url('/')},
                {'method': 'POST', 'url': test.get_url('/api'), 'data': 'test'},
                {'method': 'HEAD', 'url': test.get_url('/check')},
            ])
        """
        method_map = {
            "GET": http_module.get,
            "POST": http_module.post,
            "PUT": http_module.put,
            "DELETE": http_module.delete,
            "HEAD": http_module.head,
        }

        request_specs = []
        for idx, req in enumerate(request_list):
            method = req["method"].upper()
            if method not in method_map:
                raise ValueError(f"Unsupported HTTP method: {method}")

            func = method_map[method]
            kwargs = {
                "url": req["url"],
                "headers": req.get("headers"),
                "http_version": req.get("http_version", "1.0"),
                "timeout": req.get("timeout", 5),
            }

            # Add data for POST/PUT
            if method in ["POST", "PUT"] and "data" in req:
                kwargs["data"] = req["data"]

            request_specs.append(
                {
                    "func": func,
                    "kwargs": kwargs,
                    "id": idx,
                }
            )

        return run_concurrent_requests(request_specs, max_workers)

    return concurrent_requests
