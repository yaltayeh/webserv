import requests
from luka import logger
from .http10 import create_http10_session

def get(url, headers=None, http_version="1.1"):
    logger.info(f"GET {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.get(url, headers=headers, timeout=5)
        else:
            response = requests.get(url, headers=headers, timeout=5)
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"GET request to {url} failed: {e}")
        return None

def post(url, data=None, json=None, headers=None, http_version="1.1"):
    logger.info(f"POST {url} (HTTP/{http_version})")
    try:
        if http_version == "1.0":
            session = create_http10_session()
            response = session.post(url, data=data, json=json, headers=headers, timeout=5)
        else:
            response = requests.post(url, data=data, json=json, headers=headers, timeout=5)
        return response
    except requests.exceptions.RequestException as e:
        logger.error(f"POST request to {url} failed: {e}")
        return None
