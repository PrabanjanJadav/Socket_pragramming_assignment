import socket
import sys
import select, time

nodes = {}
originID = None
neigh = {}
time_period = 20
next_period = time.time() + time_period
LSP_state = ""

def parse_ls(msg):
    if msg[0]=='[':
        msg = msg[1:]

    n = len(msg)
    ans = []
    global neigh
    global originID
    neigh = {}
    for i in range(0,n):
        if msg[i]==',':
            continue
        if msg[i]=='(':
            j = i
            while msg[j]!=')':
                j+=1
            a, b, c, d = msg[i+1:j].split(', ')
            ans.append(a+"="+d)
            if int(d) != 0:
                neigh[a] = [b,int(c)]
            else:
                originID = a
    return ",".join(ans)

def build_lsp_pkt():
    ans = "[" + originID + ", " + str(nodes[originID][0]) + ", (" + nodes[originID][1] + ")]"
    nodes[originID][0] += 1
    return ans.encode('utf-8')

def parse_lsp_pkt(msg):
    msg = msg[1:]
    a, b, c = msg.split(", ", 2)
    b = int(b)

    if a not in nodes.keys():
        nodes[a] = [b, c[1:-2]]
        return a
    if (nodes[a][0] >= b) or (a==originID):
        return ""
    nodes[a][0] = b
    c = c[1:-2]
    nodes[a][1] = c
    return a

def print_graph():
    print("Graph received from in Virtual Node: " + originID)
    for c in nodes.keys():
        print("Source " + c + ": " + nodes[c][1])
    print()


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
tcp.send(f"({VN_IP},{UDP_PORT})".encode())
buf = b""

while True:
    timeout = max(0, next_period - time.time())
    readable, _, _ = select.select([tcp, udp], [], [], timeout)

    if tcp in readable:
        data = tcp.recv(1024)
        if not data:
            print("ON disconnected")
            sys.exit(0)
        buf += data
        while b"]" in buf:
            msg, buf = buf.split(b"]", 1)
            LSP_state = parse_ls(msg.decode())
            if originID not in nodes.keys():
                nodes[originID] = [0, ""]
            nodes[originID][1] = LSP_state
            print(LSP_state)
            msg =  build_lsp_pkt()
            for c in neigh.keys():
                udp.sendto(msg,(neigh[c][0],neigh[c][1]))

    if udp in readable:
        data, addr = udp.recvfrom(1024)
        k = parse_lsp_pkt(data.decode())
        if (k!=""):
            for c in neigh.keys():
                if (neigh[c][0], neigh[c][1]) == addr:
                    continue
                udp.sendto(data,(neigh[c][0], neigh[c][1]))

    if time.time() >= next_period:
        next_period = time.time() + time_period
        if originID is None:
            continue
        msg =  build_lsp_pkt()
        for c in neigh.keys():
            udp.sendto(msg,(neigh[c][0],neigh[c][1]))
        print_graph()