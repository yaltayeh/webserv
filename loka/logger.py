import sys

class Colors:
    RESET = "\033[0m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"
    WHITE = "\033[37m"

def log(level, message, end='\n'):
    level_colors = {
        "INFO": Colors.BLUE,
        "PASS": Colors.GREEN,
        "FAIL": Colors.RED,
        "ERROR": Colors.RED,
        "WARN": Colors.YELLOW,
    }
    color = level_colors.get(level.upper(), Colors.WHITE)
    print(f"{color}[{level.upper()}] {message}{Colors.RESET}", file=sys.stderr, end=end)

def info(message, end='\n'):
    log("INFO", message, end)

def passed(message, end='\n'):
    log("PASS", message, end)

def failed(message, end='\n'):
    log("FAIL", message, end)

def error(message, end='\n'):
    log("ERROR", message, end)

def warn(message, end='\n'):
    log("WARN", message, end)
