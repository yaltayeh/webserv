import socket
from urllib.parse import urlparse


class HTTP10Response:
    """Simple HTTP/1.0 response object mimicking requests.Response interface."""

    def __init__(self, status_code, headers, body):
        self.status_code = status_code
        self.headers = headers
        self.text = body
        self.content = body.encode("utf-8") if isinstance(body, str) else body

    def __bool__(self):
        return self.status_code < 400


def send_http10_request(method, url, data=None, headers=None):
    """
    Send a raw HTTP/1.0 request using sockets.
    Returns an HTTP10Response object mimicking requests.Response.
    """
    # Parse URL
    parsed = urlparse(url)
    host = parsed.hostname or "localhost"
    port = parsed.port or 80
    path = parsed.path or "/"
    if parsed.query:
        path += "?" + parsed.query

    # Prepare headers
    request_headers = {
        "Host": f"{host}:{port}",
        "Connection": "close",
        "User-Agent": "luka-Test-Framework/1.0",
    }
    if headers:
        request_headers.update(headers)

    # Prepare body
    body = b""
    if data:
        if isinstance(data, str):
            body = data.encode("utf-8")
        elif isinstance(data, bytes):
            body = data
        request_headers["Content-Length"] = str(len(body))

    # Build HTTP request
    request_line = f"{method.upper()} {path} HTTP/1.0\r\n"
    header_lines = "".join(f"{k}: {v}\r\n" for k, v in request_headers.items())
    request = request_line + header_lines + "\r\n"
    request_bytes = request.encode("utf-8") + body

    # Send request
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)

    try:
        sock.connect((host, port))
        sock.sendall(request_bytes)

        # Receive response
        response_data = b""
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            response_data += chunk

        sock.close()

        # Parse response
        try:
            response_text = response_data.decode("utf-8", errors="replace")
        except:
            response_text = response_data.decode("latin-1", errors="replace")

        # Split headers and body
        parts = response_text.split("\r\n\r\n", 1)
        if len(parts) < 2:
            parts = response_text.split("\n\n", 1)

        if len(parts) < 2:
            # Malformed response
            return HTTP10Response(0, {}, response_text)

        header_section, body_section = parts

        # Parse status line
        lines = header_section.split("\n")
        status_line = lines[0].strip()

        # Extract status code
        status_code = 0
        if " " in status_line:
            parts = status_line.split(" ", 2)
            if len(parts) >= 2:
                try:
                    status_code = int(parts[1])
                except ValueError:
                    status_code = 0

        # Parse headers
        response_headers = {}
        for line in lines[1:]:
            line = line.strip()
            if ":" in line:
                key, value = line.split(":", 1)
                response_headers[key.strip()] = value.strip()

        return HTTP10Response(status_code, response_headers, body_section)

    except socket.timeout:
        raise Exception(f"Connection to {host}:{port} timed out")
    except ConnectionRefusedError:
        raise Exception(f"Connection to {host}:{port} refused")
    except Exception as e:
        raise Exception(f"HTTP/1.0 request failed: {e}")
    finally:
        try:
            sock.close()
        except:
            pass


def create_http10_session():
    """
    Create a session object for HTTP/1.0 requests.
    Returns an object with get(), post(), delete(), put(), and head() methods.
    """

    class HTTP10Session:
        @staticmethod
        def get(url, headers=None, **kwargs):
            return send_http10_request("GET", url, headers=headers)

        @staticmethod
        def post(url, data=None, json=None, headers=None, **kwargs):
            if json:
                import json as json_module

                data = json_module.dumps(json)
                headers = headers or {}
                headers["Content-Type"] = "application/json"
            return send_http10_request("POST", url, data=data, headers=headers)

        @staticmethod
        def delete(url, headers=None, **kwargs):
            return send_http10_request("DELETE", url, headers=headers)

        @staticmethod
        def put(url, data=None, json=None, headers=None, **kwargs):
            if json:
                import json as json_module

                data = json_module.dumps(json)
                headers = headers or {}
                headers["Content-Type"] = "application/json"
            return send_http10_request("PUT", url, data=data, headers=headers)

        @staticmethod
        def head(url, headers=None, **kwargs):
            return send_http10_request("HEAD", url, headers=headers)

    return HTTP10Session()
