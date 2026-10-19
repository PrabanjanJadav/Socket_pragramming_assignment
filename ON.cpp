#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
#define server_ip "10.0.0.3"
#define backlog 26

class Virtual_Node {
    public:
        int client_socket;
        int udp_port;
        string client_ip;

        Virtual_Node(int c, int u, string s) {
            client_ip = s;
            udp_port = u;
            client_socket = c;
        }

        Virtual_Node() {
            client_ip = "";
            udp_port = -1;
            client_socket = -1;
        }

        void print() {
            cout << "Client with udp port as " << udp_port << " and IP address as " << client_ip << endl;
        }
};

Virtual_Node connection_msg(char* buf, int k) {
    string ip = "";
    char* p = buf;
    if(*p == '(') p++;
    while(*p != '\0') {
        if(*p != ',') {
            ip += *p;
            p++;
        }
        else {
            p++;
            break;
        }
    }
    string port = "";
    while(*p != ')') {
        port += *p;
        p++;
    }
    return Virtual_Node(k, stoi(port), ip);
}

int main(int argc, char* argv[]) {
    if(argc <= 1) {
        cout << "No file name specified" << endl;
        return -1;
    }

    string file_name = argv[1];
    ifstream file(file_name);

    if(!file.is_open()) {
        cout << "The file is not open" << endl;
        return -1;
    }

    string line;
    int N = -1, i = 0;
    vector<vector<int>> Adj(26, vector<int>(26,-1));

    while(getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if(line.empty() || line[0]=='#') {
            continue;
        }
        // cout << line << "--" << endl;

        stringstream ss(line);
        string t;
        int j = i+1;
        while(getline(ss, t, '\t')) {
            if(t.empty()) {
                continue;
            }
            // cout << t << endl;
            Adj[i][j] = stoi(t);
            j+=1;
        }

        // cout << line << endl;
        i+=1;
    }
    N = i+1;

    for(int i=0; i<26; i++) {
        for(int j=0; j<26; j++) {
            if(i==j) {
                Adj[i][j] = 0;
                continue;
            }
            else if(i > j) {
                Adj[i][j] = Adj[j][i];
            }
        }
    }

    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            cout << Adj[i][j] << " ";
        }
        cout << endl;
    }
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        cout << "Error in socket creation" << endl;
        return -1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr))<0) {
        cout << "Bind failed" << endl;
        close(server_socket);
        return -1;
    }

    if(listen(server_socket, backlog) < 0)  {
        cout << "Listen failed" << endl;
        close(server_socket);
        return -1;
    }
    i = 0;
    vector<Virtual_Node> M(26);
    while(i<N) {
        sockaddr_in client_addr;
        socklen_t len_client = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, (socklen_t*)(&len_client));
        if (client_socket < 0) {
            cout << "Failed to accept client connection";
            return -1;
        }

        char buffer[1024] = {0};
        int b = recv(client_socket, buffer, sizeof(buffer)-1, 0);
        if (b > 0) {
            M[i] = connection_msg(buffer, client_socket);
            M[i].print();
            i += 1;
        }
    }

    while(true) {
        for(int k=0; k<26; k++) {
            if(M[k].client_socket == -1) continue;
            string out = "["; 
            for(int j=0; j<26; j++) {
                if(Adj[k][j]!=-1) {
                    if(out!="") out += ",";
                    out += "(" + string(1, (char)('A'+j)) + ", " + M[j].client_ip + ", " + to_string(M[j].udp_port) + ", " + to_string(Adj[k][j]) + ")";
                }
            }
            out += "]";
            size_t sent = 0;
            while (sent < out.size()) {
                ssize_t n = send(M[k].client_socket, out.c_str() + sent, out.size() - sent, 0);
                if(n <= 0) {
                    cout << "Error no LSP message sent" << endl;
                    return -1;
                }
                sent += n;
            }
        }
        this_thread::sleep_for(chrono::seconds(10));
    }
    

    for(int j=0; j<26; j++) {
        if(M[j].client_socket >= 0) close(M[j].client_socket);
    }

    close(server_socket);
    file.close();
}