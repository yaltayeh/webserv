from loka import logger

class AssertEqualError(AssertionError):
    def __init__(self, expected, actual):
        self.expected = expected
        self.actual = actual
        super().__init__(f"Expected: {expected}, Actual: {actual}")

def assertEqual(expected, actual, message=""):
    if actual != expected:
        logger.failed(f"Assertion Failed: {message}")
        raise AssertEqualError(expected, actual)
