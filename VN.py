import socket
import sys
if len(sys.argv) != 4 :
    print("Please give atleast four argument : on_ip, vn_ip, udp_pn")
    sys.exit(1)
ON_IP = sys.argv[1]
VN_IP = sys.argv[2]
UDP_PORT = int(sys.argv[3])
udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    udp.bind((VN_IP, UDP_PORT))
except socket.error as e:
    print("Failure to bind or port already in use" + str(e))
    sys.exit(1)
PORT = 5000
tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp.connect((ON_IP, PORT))
tcp.send(f"{VN_IP},{UDP_PORT}".encode())
buf = b""
while True:
    data = tcp.recv(1024)
    if not data :
        break
    buf += data

msg = buf.decode().strip()
print(msg)