#!/usr/bin/env python3

import cgi
import cgitb
import json
import os
import sys
from datetime import datetime
import hashlib
import secrets

cgitb.enable()

# Configuration
DATA_DIR = "../data"
USERS_FILE = "users.json"
SESSIONS_FILE = "sessions.json"


def send_json_with_cookie(data, cookie=None):
    """Send JSON response with optional cookie"""
    print("Content-Type: application/json")
    if cookie:
        print(f"Set-Cookie: {cookie}")
    print()
    print(json.dumps(data, indent=2))


def get_data_file(filename):
    """Get path to data file"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_path = os.path.join(script_dir, DATA_DIR)

    if not os.path.exists(data_path):
        os.makedirs(data_path)

    return os.path.join(data_path, filename)


def load_users():
    """Load users from JSON file"""
    users_file = get_data_file(USERS_FILE)

    if os.path.exists(users_file):
        try:
            with open(users_file, "r") as f:
                return json.load(f)
        except:
            return {}
    return {}


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


def hash_password(password):
    """Simple password hashing"""
    return hashlib.sha256(password.encode()).hexdigest()


def create_session(username):
    """Create a new session for user"""
    token = secrets.token_hex(32)
    sessions = load_sessions()

    sessions[token] = {
        "username": username,
        "created": datetime.now().isoformat(),
        "last_access": datetime.now().isoformat(),
    }

    save_sessions(sessions)
    return token


def main():
    # Parse form data
    form = cgi.FieldStorage()

    username = form.getvalue("username", "").strip()
    password = form.getvalue("password", "")

    # Validate input
    if not username or not password:
        send_json_with_cookie(
            {"success": False, "message": "Username and password are required"}
        )
        return

    # Check credentials
    users = load_users()

    if username not in users:
        send_json_with_cookie(
            {"success": False, "message": "Invalid username or password"}
        )
        return

    user_data = users[username]
    password_hash = hash_password(password)

    if user_data["password"] != password_hash:
        send_json_with_cookie(
            {"success": False, "message": "Invalid username or password"}
        )
        return

    # Create session
    token = create_session(username)

    # Set cookie (expires in 24 hours)
    cookie = f"session_token={token}; Path=/; Max-Age=86400; SameSite=Lax"

    send_json_with_cookie(
        {"success": True, "message": "Login successful", "username": username}, cookie
    )


if __name__ == "__main__":
    main()
