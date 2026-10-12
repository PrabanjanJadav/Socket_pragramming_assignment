#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
#define server_ip "10.2.44.57"
#define backlog 26

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
    vector<vector<int>> Adj(N, vector<int>(N));

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
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    memset(&(server_addr), '\0', 8);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))<0) {
        cout << "Bind failed" << endl;
        close(server_socket);
        return -1;
    }

    if(listen(server_socket, backlog) < 0)  {
        cout << "Listen failed" << endl;
        close(server_socket);
        return -1;
    }

    


    file.close();
}