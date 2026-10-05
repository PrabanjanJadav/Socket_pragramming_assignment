import socket
server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind(("127.0.0.1", 12345))
print("Server running...")
while True:
    data, client_addr = server.recvfrom(1024)
    print("Received:", data.decode(), "from", client_addr)
    print(data, client_addr)
    server.sendto(b"Hello from server", client_addr)
