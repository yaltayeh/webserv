#!/usr/bin/env python3
import cgi
import os

UPLOAD_DIR = "/data/uploads"

form = cgi.FieldStorage()

fileitem = form['file']

if fileitem.filename:
    filename = os.path.basename(fileitem.filename)
    path = os.path.join(UPLOAD_DIR, filename)

    with open(path, 'wb') as f:
        while True:
            chunk = fileitem.file.read(1024 * 1024)
            if not chunk:
                break
            f.write(chunk)

    print("Content-Type: text/plain\n")
    print("Upload successful")
else:
    print("Content-Type: text/plain\n")
    print("No file uploaded")
