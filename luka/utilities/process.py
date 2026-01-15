import subprocess
import os
from luka import logger

def run_command(command, cwd=None, wait=True):
    # logger.info(f"Executing: {' '.join(command)}")
    try:
        process = subprocess.Popen(
            command,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        if not wait:
            return process

        stdout, stderr = process.communicate()
        if process.returncode != 0:
            logger.error(f"Command failed with exit code {process.returncode}")
            logger.error(f"Stderr: {stderr}")
        return process.returncode, stdout, stderr
    except FileNotFoundError:
        logger.error(f"Command not found: {command[0]}")
        return None, "", f"Command not found: {command[0]}"
    except Exception as e:
        logger.error(f"An unexpected error occurred: {e}")
        return None, "", str(e)
