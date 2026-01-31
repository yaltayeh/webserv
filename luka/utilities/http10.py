import socket
from urllib.parse import urlparse


class HTTP10Response:
    """Simple HTTP/1.0 response object mimicking requests.Response interface."""

    def __init__(self, status_code, headers, body_bytes):
        self.status_code = status_code
        self.headers = headers
        self.content = (
            body_bytes if isinstance(body_bytes, bytes) else body_bytes.encode("utf-8")
        )
        # Lazy decode text only when accessed
        self._text = None

    @property
    def text(self):
        if self._text is None:
            try:
                self._text = self.content.decode("utf-8")
            except:
                self._text = self.content.decode("latin-1", errors="replace")
        return self._text

    def __bool__(self):
        return self.status_code < 400


def send_http10_request(method, url, data=None, headers=None, timeout=5):
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
    sock.settimeout(timeout)

    try:
        sock.connect((host, port))
        sock.sendall(request_bytes)

        # Receive response
        response_data = b""
        total_received = 0
        while True:
            try:
                chunk = sock.recv(262144)  # 256KB buffer for faster downloads
                if not chunk:
                    break
                response_data += chunk
                total_received += len(chunk)
            except socket.timeout:
                # If we got some data, it might be incomplete but let's try to parse it
                if response_data:
                    break
                raise
            except ConnectionResetError:
                # Server closed connection - this can be normal for HTTP/1.0
                # Continue with whatever data we have
                break

        sock.close()

        # Check if we got any response
        if not response_data:
            raise Exception("Server closed connection without sending any data")

        # Split headers and body at the binary level (not after decoding)
        # This prevents corruption of binary data
        header_end = response_data.find(b"\r\n\r\n")
        if header_end == -1:
            header_end = response_data.find(b"\n\n")
            if header_end == -1:
                # Malformed response - try to decode and return what we have
                try:
                    response_text = response_data.decode("utf-8", errors="replace")
                except:
                    response_text = response_data.decode("latin-1", errors="replace")
                return HTTP10Response(0, {}, response_text.encode("latin-1"))
            header_bytes = response_data[:header_end]
            body_bytes = response_data[header_end + 2 :]  # Skip \n\n
        else:
            header_bytes = response_data[:header_end]
            body_bytes = response_data[header_end + 4 :]  # Skip \r\n\r\n

        # Decode headers only (safe to decode as text)
        try:
            header_section = header_bytes.decode("utf-8", errors="replace")
        except:
            header_section = header_bytes.decode("latin-1", errors="replace")

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

        # Return response with binary body
        return HTTP10Response(status_code, response_headers, body_bytes)

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
        def get(url, headers=None, timeout=5, **kwargs):
            return send_http10_request("GET", url, headers=headers, timeout=timeout)

        @staticmethod
        def post(url, data=None, json=None, headers=None, timeout=5, **kwargs):
            if json:
                import json as json_module

                data = json_module.dumps(json)
                headers = headers or {}
                headers["Content-Type"] = "application/json"
            return send_http10_request(
                "POST", url, data=data, headers=headers, timeout=timeout
            )

        @staticmethod
        def delete(url, headers=None, timeout=5, **kwargs):
            return send_http10_request("DELETE", url, headers=headers, timeout=timeout)

        @staticmethod
        def put(url, data=None, json=None, headers=None, timeout=5, **kwargs):
            if json:
                import json as json_module

                data = json_module.dumps(json)
                headers = headers or {}
                headers["Content-Type"] = "application/json"
            return send_http10_request(
                "PUT", url, data=data, headers=headers, timeout=timeout
            )

        @staticmethod
        def head(url, headers=None, timeout=5, **kwargs):
            return send_http10_request("HEAD", url, headers=headers, timeout=timeout)

    return HTTP10Session()
