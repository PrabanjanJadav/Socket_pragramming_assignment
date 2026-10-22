import socket
import sys
import select, time
import numpy as np
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

def nodes_to_graph(nodes):
    names = set(nodes)
    for value in nodes.values():
        for e in value[1].split(","):
            names.add(e.split("=")[0])
    names = sorted(names)
    n = len(names)

    graph_matrix = np.full((n, n), -1)
    idx = {}
    for i in range(n):
        idx[names[i]] = i
    for key, value in nodes.items():
        i = idx[key]
        graph_matrix[i][i] = 0
        indi = value[1].split(",")
        for e in indi:
            fin = e.split("=")
            graph_matrix[i][idx[fin[0]]] = int(fin[1])
            # print(key + " to " + fin[0] + " = " + fin[1])
    return graph_matrix

if len(sys.argv) != 4 :
    print("Please give atleast three argument : on_ip, vn_ip, udp_pn")
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
try:
    tcp.connect((ON_IP, PORT))
except socket.error as e:
    print(f"Cannot connect to ON at {ON_IP}:{PORT} ({e})")
    sys.exit(1)

tcp.send(f"({VN_IP},{UDP_PORT})".encode('utf-8'))
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
                try:
                    udp.sendto(msg,(neigh[c][0],neigh[c][1]))
                except socket.error as e:
                    print(f"Error in msg reading from server {e}")
                    # sys.exit(1)

    if udp in readable:
        data, addr = udp.recvfrom(1024)
        try:
            k = parse_lsp_pkt(data.decode())
        except Exception:
            k = ""
        if (k!=""):
            for c in neigh.keys():
                if (neigh[c][0], neigh[c][1]) == addr:
                    continue
                try:
                    udp.sendto(data,(neigh[c][0], neigh[c][1]))
                except socket.error as e:
                    print(f"Message not sent to neighbour {e}")
                    # sys.exit(1)

    if time.time() >= next_period:
        next_period = time.time() + time_period
        if originID is None:
            continue
        msg =  build_lsp_pkt()
        for c in neigh.keys():
            udp.sendto(msg,(neigh[c][0],neigh[c][1]))
        print(nodes_to_graph(nodes))