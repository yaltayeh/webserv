import sys
from time import sleep

print("HTTP/1.1 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n')
content = "Python CGI index"
print("Content-Length: ", len(content), end='\r\n')

print(end='\r\n')


print(content)
sleep(10) # sleep 10 seconds to simulate long processing 