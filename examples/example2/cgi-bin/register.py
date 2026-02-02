#!/usr/bin/env python3

import cgi
import cgitb
import json
import os
import sys
from datetime import datetime
import hashlib

cgitb.enable()

# Configuration
DATA_DIR = "../data"
USERS_FILE = "users.json"


def send_json(data):
    """Send JSON response"""
    print("Content-Type: application/json")
    print()
    print(json.dumps(data, indent=2))


def get_users_file():
    """Get path to users file"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_path = os.path.join(script_dir, DATA_DIR)

    if not os.path.exists(data_path):
        os.makedirs(data_path)

    return os.path.join(data_path, USERS_FILE)


def load_users():
    """Load users from JSON file"""
    users_file = get_users_file()

    if os.path.exists(users_file):
        try:
            with open(users_file, "r") as f:
                return json.load(f)
        except:
            return {}
    return {}


def save_users(users):
    """Save users to JSON file"""
    users_file = get_users_file()
    with open(users_file, "w") as f:
        json.dump(users, f, indent=2)


def hash_password(password):
    """Simple password hashing"""
    return hashlib.sha256(password.encode()).hexdigest()


def main():
    # Parse form data
    form = cgi.FieldStorage()

    username = form.getvalue("username", "").strip()
    email = form.getvalue("email", "").strip()
    password = form.getvalue("password", "")

    # Validate input
    if not username or not email or not password:
        send_json({"success": False, "message": "All fields are required"})
        return

    if len(username) < 3:
        send_json(
            {"success": False, "message": "Username must be at least 3 characters"}
        )
        return

    if len(password) < 6:
        send_json(
            {"success": False, "message": "Password must be at least 6 characters"}
        )
        return

    # Check if user already exists
    users = load_users()

    if username in users:
        send_json({"success": False, "message": "Username already exists"})
        return

    # Check if email already exists
    for user_data in users.values():
        if user_data.get("email") == email:
            send_json({"success": False, "message": "Email already registered"})
            return

    # Create new user
    users[username] = {
        "email": email,
        "password": hash_password(password),
        "registered": datetime.now().isoformat(),
    }

    save_users(users)

    send_json(
        {"success": True, "message": "Registration successful", "username": username}
    )


if __name__ == "__main__":
    main()
