#!/usr/bin/env python3

import cgi
import cgitb
import os
import sys
import json
from datetime import datetime

cgitb.enable()

# Configuration
UPLOAD_DIR = "../uploads"
MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB


def send_response(success, message, filename=None):
    """Send JSON response"""
    response = {
        "success": success,
        "message": message,
        "filename": filename,
        "timestamp": datetime.now().isoformat(),
    }

    print("Content-Type: application/json")
    print()
    print(json.dumps(response, indent=2))


def main():
    # Ensure upload directory exists
    script_dir = os.path.dirname(os.path.abspath(__file__))
    upload_path = os.path.join(script_dir, UPLOAD_DIR)

    if not os.path.exists(upload_path):
        os.makedirs(upload_path)

    # Parse form data
    form = cgi.FieldStorage()

    # Check if file was uploaded
    if "file" not in form:
        send_response(False, "No file was uploaded")
        return

    fileitem = form["file"]

    # Check if file is valid
    if not fileitem.filename:
        send_response(False, "No file selected")
        return

    # Get optional description
    description = form.getvalue("description", "")

    # Sanitize filename
    filename = os.path.basename(fileitem.filename)
    filename = "".join(c for c in filename if c.isalnum() or c in "._- ")

    if not filename:
        send_response(False, "Invalid filename")
        return

    # Check for duplicate and generate unique name if needed
    base_name, extension = os.path.splitext(filename)
    counter = 1
    unique_filename = filename
    filepath = os.path.join(upload_path, unique_filename)

    while os.path.exists(filepath):
        unique_filename = f"{base_name}_{counter}{extension}"
        filepath = os.path.join(upload_path, unique_filename)
        counter += 1

    try:
        # Write file to disk
        with open(filepath, "wb") as f:
            chunk_size = 8192
            while True:
                chunk = fileitem.file.read(chunk_size)
                if not chunk:
                    break
                f.write(chunk)

        # Get file size
        file_size = os.path.getsize(filepath)

        if file_size > MAX_FILE_SIZE:
            os.remove(filepath)
            send_response(
                False, f"File too large (max {MAX_FILE_SIZE // (1024 * 1024)}MB)"
            )
            return

        # Save metadata
        metadata_file = os.path.join(upload_path, ".metadata.json")
        metadata = {}

        if os.path.exists(metadata_file):
            with open(metadata_file, "r") as f:
                metadata = json.load(f)

        metadata[unique_filename] = {
            "original_name": filename,
            "description": description,
            "size": file_size,
            "upload_time": datetime.now().isoformat(),
        }

        with open(metadata_file, "w") as f:
            json.dump(metadata, f, indent=2)

        send_response(
            True, f"File uploaded successfully as '{unique_filename}'", unique_filename
        )

    except Exception as e:
        send_response(False, f"Error uploading file: {str(e)}")


if __name__ == "__main__":
    main()
