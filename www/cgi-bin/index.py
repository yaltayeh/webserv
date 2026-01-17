from time import sleep


print("HTTP/1.1 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n')
content = "Python CGI index"
print("Content-Length: ", len(content), end='\r\n')

print(end='\r\n')

sleep(60 * 3) # sleep 3 minutes 

print(content ,end='\r\n')