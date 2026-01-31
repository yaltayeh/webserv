import sys


# Global flag to suppress output
QUIET_MODE = False


def set_quiet(enabled):
    """Enable or disable quiet mode (suppresses all logging)."""
    global QUIET_MODE
    QUIET_MODE = enabled


class Colors:
    RESET = "\033[0m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"
    WHITE = "\033[37m"
    ORANGE = "\033[38;5;208m"


def log(level, message, end="\n"):
    if QUIET_MODE:
        return
    level_colors = {
        "INFO": Colors.BLUE,
        "PASS": Colors.GREEN,
        "FAIL": Colors.RED,
        "ERROR": Colors.RED,
        "WARN": Colors.YELLOW,
    }
    color = level_colors.get(level.upper(), Colors.WHITE)
    print(f"{color}[{level.upper()}] {message}{Colors.RESET}", file=sys.stderr, end=end)


def info(message, end="\n"):
    log("INFO", message, end)


def passed(message, end="\n"):
    log("PASS", message, end)


def failed(message, end="\n"):
    log("FAIL", message, end)


def error(message, end="\n"):
    log("ERROR", message, end)


def warn(message, end="\n"):
    log("WARN", message, end)


def orange(message, end="\n"):
    if QUIET_MODE:
        return
    print(f"{Colors.ORANGE}{message}{Colors.RESET}", file=sys.stderr, end=end)


# Compact mode functions
def print_category(category_name, end=""):
    """Print category name in compact mode format."""
    print(
        f"{Colors.CYAN}[{category_name}]{Colors.RESET} ",
        file=sys.stderr,
        end=end,
        flush=True,
    )


def print_check(end=" "):
    """Print green checkmark for passed test."""
    print(f"{Colors.GREEN}✓{Colors.RESET}", file=sys.stderr, end=end, flush=True)


def print_cross(end=" "):
    """Print red cross for failed test."""
    print(f"{Colors.RED}⨯{Colors.RESET}", file=sys.stderr, end=end, flush=True)


def print_leak_cross(end=" "):
    """Print orange cross for test with valgrind leaks."""
    print(f"{Colors.ORANGE}⨯{Colors.RESET}", file=sys.stderr, end=end, flush=True)
