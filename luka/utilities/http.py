import requests
from luka import logger
from .http10 import create_http10_session


def get(url, headers=None, http_version="1.1", timeout=5):
    logger.info(f"GET {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.get(url, headers=headers, timeout=timeout)
        else:
            response = requests.get(url, headers=headers, timeout=timeout)
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"GET request to {url} failed: {e}")
        return None


def post(url, data=None, json=None, headers=None, http_version="1.1"):
    logger.info(f"POST {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.post(
                url, data=data, json=json, headers=headers, timeout=5
            )
        else:
            response = requests.post(
                url, data=data, json=json, headers=headers, timeout=5
            )
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"POST request to {url} failed: {e}")
        return None


def delete(url, headers=None, http_version="1.1"):
    logger.info(f"DELETE {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.delete(url, headers=headers, timeout=5)
        else:
            response = requests.delete(url, headers=headers, timeout=5)
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"DELETE request to {url} failed: {e}")
        return None


def put(url, data=None, json=None, headers=None, http_version="1.1"):
    logger.info(f"PUT {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.put(
                url, data=data, json=json, headers=headers, timeout=5
            )
        else:
            response = requests.put(
                url, data=data, json=json, headers=headers, timeout=5
            )
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"PUT request to {url} failed: {e}")
        return None


def head(url, headers=None, http_version="1.1", timeout=5):
    logger.info(f"HEAD {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.head(url, headers=headers, timeout=timeout)
        else:
            response = requests.head(url, headers=headers, timeout=timeout)
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"HEAD request to {url} failed: {e}")
        return None


def calculate_timeout(size_mb, min_timeout=30, speed_mbps=10):
    """
    Calculate dynamic timeout based on file size.

    Args:
        size_mb: File size in megabytes
        min_timeout: Minimum timeout in seconds (default: 30)
        speed_mbps: Expected download speed in Mbps (default: 10)

    Returns:
        Timeout in seconds
    """
    # Convert Mbps to MB/s: speed_mbps / 8
    # Time = size / speed, add 10 second buffer
    calculated = int(size_mb / (speed_mbps / 8) + 10)
    return max(min_timeout, calculated)
