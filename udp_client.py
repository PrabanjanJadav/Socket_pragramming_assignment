import socket
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.sendto(b"Hello from client", ("127.0.0.1", 12345))
data, addr = client.recvfrom(1024)
print(addr, data)
print("Reply:", data.decode())
client.close()
