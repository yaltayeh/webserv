import socket
from luka import logger

def is_port_open(host, port):
    """
    Check if a TCP port is open on a given host.
    """
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(0.1)  # Don't wait too long
    try:
        s.connect((host, port))
        s.close()
        return True
    except (socket.timeout, ConnectionRefusedError):
        return False
