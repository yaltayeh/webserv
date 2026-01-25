import platform

print("Status: 200", end='\r\n')
print("Content-Type: text/html", end='\r\n')

print(end='\r\n')

system = platform.uname()

print(f"System: {system.system}")
print(f"Node Name: {system.node}")
print(f"Release: {system.release}")
print(f"Version: {system.version}")
print(f"Machine: {system.machine}")
print(f"Processor: {system.processor}")
