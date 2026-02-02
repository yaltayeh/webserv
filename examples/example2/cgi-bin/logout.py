#!/usr/bin/env python3

import cgi
import cgitb
import json
import os
import sys
from datetime import datetime

cgitb.enable()

# Configuration
DATA_DIR = "../data"
SESSIONS_FILE = "sessions.json"


def send_json(data):
    """Send JSON response"""
    print("Content-Type: application/json")
    print()
    print(json.dumps(data, indent=2))


def get_data_file(filename):
    """Get path to data file"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_path = os.path.join(script_dir, DATA_DIR)

    if not os.path.exists(data_path):
        os.makedirs(data_path)

    return os.path.join(data_path, filename)


def load_sessions():
    """Load sessions from JSON file"""
    sessions_file = get_data_file(SESSIONS_FILE)

    if os.path.exists(sessions_file):
        try:
            with open(sessions_file, "r") as f:
                return json.load(f)
        except:
            return {}
    return {}


def save_sessions(sessions):
    """Save sessions to JSON file"""
    sessions_file = get_data_file(SESSIONS_FILE)
    with open(sessions_file, "w") as f:
        json.dump(sessions, f, indent=2)


def get_session_token():
    """Extract session token from cookies"""
    cookie_string = os.environ.get("HTTP_COOKIE", "")

    if not cookie_string:
        return None

    cookies = cookie_string.split(";")
    for cookie in cookies:
        cookie = cookie.strip()
        if cookie.startswith("session_token="):
            return cookie.split("=", 1)[1]

    return None


def main():
    token = get_session_token()

    if not token:
        send_json({"success": False, "message": "No session token found"})
        return

    # Load sessions and remove this token
    sessions = load_sessions()

    if token not in sessions:
        send_json({"success": False, "message": "Invalid session"})
        return

    # Remove session
    del sessions[token]
    save_sessions(sessions)

    # Clear cookie
    print("Content-Type: application/json")
    print("Set-Cookie: session_token=; Path=/; Max-Age=0")
    print()
    print(json.dumps({"success": True, "message": "Logged out successfully"}, indent=2))


if __name__ == "__main__":
    main()
