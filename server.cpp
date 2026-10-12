#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
#define server_ip "127.0.0.8"
#define backlog 26

vector<string> connection_msg(char* buf) {
    string ip = "";
    char* p = buf;
    while(*p != '\0') {
        if(*p != ',') {
            ip += *p;
            p++;
        }
        else break;
    }
    string port = "";
    while(*p != '\0') {
        port += *p;
        p++;
    }
    return {ip, port};
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
    vector<vector<int>> A;
    int N = -1;

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
        vector<int> B;
        while(getline(ss, t, '\t')) {
            if(t.empty()) {
                B.push_back(-1);
                continue;
            }
            // cout << t << endl;
            B.push_back(stoi(t));
        }
        A.push_back(B);

        // cout << line << endl;
    }
    N = A.size() + 1;
    vector<vector<int>> Adj(26, vector<int>(26,-1));

    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            if(i==j) {
                Adj[i][j] = 0;
                continue;
            }
            else if(i < j) {
                Adj[i][j] = A[i][j-1];
            }
            else {
                Adj[i][j] = A[j][i-1];
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
    int i = 0;
    vector<vector<string>> M(26, vector<string>(2));

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
            M[i] = connection_msg(buffer);
            cout << M[i][0] << " " << M[i][1] << endl;
            i += 1;
        }
        string out = string(1, (char)('A'+i- 1)) + ", " + M[i-1][0] + ", " + M[i-1][1] + ", 0";

        size_t sent = 0;
        while (sent < out.size()) {
            ssize_t n = send(client_socket, out.c_str() + sent, out.size() - sent, 0);
            sent += n;
        }

        close(client_socket);

    close(server_socket);
    file.close();
}