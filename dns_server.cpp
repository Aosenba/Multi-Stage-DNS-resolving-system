#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

vector<vector<string>> database;

void dns2proxy(int sockfd){
    char buffer[256];

    // Receiving the request from client 
    int bytes_recv = recv(sockfd, buffer, sizeof(buffer), 0);
    string s = string(buffer);
    int length = stoi(s.substr(0,2));
    int type = s[2] - '0';
    string request = string(buffer + 3);
    request = request.substr(0, length-1);

    cout << "Length: " << length << "\n";
    cout << "Type of the request: " << type << "\n";
    cout << "Request: " << request << "\n";

    // Reading the cache and storing it in a vector
    int entry_cnt = 0; // number of entries in the cache already present
    ifstream in;
    in.open("dns.txt", ios::in);
    string first, second;
    while(in >> first >> second){
        if(first == "" || second == "") continue;
        database.push_back({first, second});
        entry_cnt ++;
    }
    in.close();
    // Search if the request from client exists in the proxy cache
    // Type 1: Request for IP -> Search for mapping DOMAIN
    // Type 2: Request for Domain -> Search for mapping IP
    bool hit = false;
    string response;
    for(int i=0; i<entry_cnt; ++i){
        if(database[i][type-1] == request){
            hit = true;
            response = "3" + database[i][type%2];
            break;
        }
    }

    if(!hit){
        cout << "DNS resolution failed.\n";
        response = "4entry not found in database";
    }

    send(sockfd, response.c_str(), 256, 0);
    close(sockfd);
}

int main(int argc, char *argv[]){
    // Creating the socket
    int sockfd = 0;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else{
        cout << "Socket created succesfully.\n";
    }

    // Binding the socket to localhost
    // Setting the proxy address
    int proxy_port = stoi(string(argv[1]));
    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // not using INADDR_ANY because it will bind the socket to all the available interfaces including the localhost
    proxy_addr.sin_port = htons(proxy_port); 
    
    if(bind(sockfd, (struct sockaddr*) &proxy_addr, sizeof(proxy_addr)) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else{
        cout << "Socket binded to the localhost and port " << proxy_port << " successfully.\n";
    }

    // Listen to the port for any incoming messages from the client
    // listen(sockfd, backlog) where backlog defines the size of the queue to which the length of pending messages may grow
    if(listen(sockfd, 5) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else {
        cout << "DNS is in LISTEN state.\n";
    }

    listen:
    int new_sockfd = 0, connection_cnt = 0;
    socklen_t proxy_addr_size = sizeof(proxy_addr);
    if((new_sockfd = accept(sockfd, (struct sockaddr*) &proxy_addr, &proxy_addr_size)) < 0){
        cout << "Accept failed!\n";
    }
    else {
        connection_cnt ++;
        cout << "Connetion " << connection_cnt << " accepted from the client.\n";
    }

    // Function thread that takes care of further incoming connections
    int pid =  fork();
    if(pid == 0)
        dns2proxy(new_sockfd);
    else
        goto listen;
    
    // Free socket
    close(sockfd);
    return 0;
}