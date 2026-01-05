import requests

URL = "http://localhost:12345"

for i in range(10000):
    r = requests.get(URL)
    print(i, r.status_code)
