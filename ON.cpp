#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
#define server_ip "10.34.214.240"
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
    while(*p != ')' && *p != '\0') {
        port += *p;
        p++;
    }
    return Virtual_Node(k, stoi(port), ip);
}

string read_file(const string& p) {
    ifstream f(p);
    stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool load_config(const string& path, vector<vector<int>>& Adj, int& N) {
    string line;
    int i = 0;
    ifstream file(path);
    if(!file.is_open()) return false;

    Adj = vector<vector<int>>(26, vector<int>(26,-1));

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
        while(ss >> t) {
            if(t.empty()) {
                continue;
            }
            // cout << t << endl;
            try {
                Adj[i][j] = stoi(t);
            }
            catch(...) {
                cout << "Issue in Stoi" << endl;
                return false;
            }
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

    cout << "[";
    for(int i=0; i<N; i++) {
        cout << "[";
        for(int j=0; j<N; j++) {
            cout << Adj[i][j] << " ";
        }
        if(i < N-1) cout << "]" << endl;
        else cout << "]";
    }
    cout << "]" << endl;
    return true;
}

void send_link_state(vector<vector<int>>& Adj, int k, vector<Virtual_Node>& M) {
    if(M[k].client_socket == -1) return;
    string out = "[";
    for(int j=0; j<26; j++) {
        if(Adj[k][j]!=-1) {
            if(out.size() > 1) out += ",";
            out += "(" + string(1, (char)('A'+j)) + ", " + M[j].client_ip + ", " + to_string(M[j].udp_port) + ", " + to_string(Adj[k][j]) + ")";
        }
    }
    out += "]";
    size_t sent = 0;
    while (sent < out.size()) {
        ssize_t n = send(M[k].client_socket, out.data() + sent, out.size() - sent, 0);
        if(n <= 0) {
            cout << "Error no LSP message sent" << endl;
            break;
        }
        sent += n;
    }
    return;
}

int main(int argc, char* argv[]) {
    if(argc <= 1) {
        cout << "No file name specified" << endl;
        return -1;
    }

    string file_name = argv[1];
    int N = -1, i = 0;
    vector<vector<int>> Adj(26, vector<int>(26,-1));

    if(!load_config(file_name, Adj, N)) { 
        cout << "Could not read config" << endl; 
        return -1; 
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
    if(argc <= 2) {
        server_addr.sin_addr.s_addr = inet_addr(server_ip);
    }
    else {
        server_addr.sin_addr.s_addr = inet_addr(argv[2]);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    for(int k=0; k<26; k++) send_link_state(Adj, k, M);
    string last = read_file(file_name);
    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    while(true) {
        this_thread::sleep_for(chrono::seconds(1));
        int x = accept(server_socket, NULL, NULL);
        if(x >= 0 && i < 26) {
            timeval t = {2, 0};
            setsockopt(x, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
            char buffer[1024] = {0};
            int b = recv(x, buffer, sizeof(buffer)-1, 0);
            if (b > 0) {
                M[i] = connection_msg(buffer, x);
                M[i].print();
                send_link_state(Adj, i, M);
                i += 1;
            }
            else close(x);
        }
        else if(x >= 0) close(x);

        string cur = read_file(file_name);
        if(cur.empty() || cur == last) continue;
        last = cur;
        vector<vector<int>> tem_adj;
        int temp;
        if(load_config(file_name, tem_adj, temp)) { 
            Adj = tem_adj;
            N = temp;
            for(int k=0; k<26; k++) send_link_state(Adj, k, M);
        }
        else {
            cout << "Could not read config or some error, using old matrix" << endl; 
        }
    }
    

    for(int j=0; j<26; j++) {
        if(M[j].client_socket >= 0) close(M[j].client_socket);
    }

    close(server_socket);
}