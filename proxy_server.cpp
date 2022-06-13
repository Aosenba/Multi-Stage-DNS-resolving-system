#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>

using namespace std;


// Constant IP and PORT of DNS known to the proxy
static const char DNS_IP[INET_ADDRSTRLEN] = "127.0.0.1";
static const int CACHE_SIZE = 3;

vector<vector<string>> cache;


string proxy2dns(char *send_buffer){
    // Creating socket to send request to DNS
    int sockfd = 0;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else{
        cout << "Socket to connet to DNS created.\n";
    }
    
    // Enter the DNS IP and PORT
    int DNS_PORT = 0;
    cout << "<DNS_PORT>\n";
    cin >> DNS_PORT;
    
    // Connecting to socket of DNS
    struct sockaddr_in dns_addr;
    dns_addr.sin_family = AF_INET;
    dns_addr.sin_addr.s_addr = inet_addr(DNS_IP);
    dns_addr.sin_port = htons(DNS_PORT);

    if(connect(sockfd, (struct sockaddr*) &dns_addr, sizeof(dns_addr)) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else{
        cout << "Connection with DNS is established.\n";
    }

    // Forward the cache miss request of proxy to DNS
    send(sockfd, send_buffer, 256, 0);

    // Getting and parsing the response from the DNS
    char recv_buffer[256];
    int bytes_recv = 0;
    
    // Receiving the response from the DNS
    if((bytes_recv = recv(sockfd, recv_buffer, 256, 0)) < 0){
        cout << "DNS did not respond.\n";
        exit(0);
    }

    int type = recv_buffer[0] - '0';
    string response = string(recv_buffer + 1);

    // If the requested DOMAIN/IP exists in the DNS then update the cache
    string s = string(send_buffer);
    int length = stoi(s.substr(0,2));
    string request = string(send_buffer + 3);
    request = request.substr(0, length-1);
    string first = send_buffer[2] - '0' == 1 ? request : response;
    string second = send_buffer[2] - '0' == 1 ? response : request; 
    
    if(type == 3){
        if(cache.size() < CACHE_SIZE){
            cache.push_back({first, second});
        }
        else{
            // Apply FIFO to replace the entry in cache and insert the new response from DNS in cache
            for(int i=1; i<CACHE_SIZE; ++i){
                cache[i-1][0] = cache[i][0];
                cache[i-1][1] = cache[i][1];
            }
            cache[CACHE_SIZE-1][0] = first;
            cache[CACHE_SIZE-1][1] = second;
        }
        // Write to the proxy_cache.txt
        ofstream out;
        out.open("proxy_cache.txt", ofstream::out | ofstream::trunc);
        int i;
        for(i=0; i<cache.size()-1; ++i){
            out << cache[i][0] << " " << cache[i][1] << "\n";
        }
        out << cache[i][0] << " " << cache[i][1];
        out.close();
    }

    // Close connection with DNS
    close(sockfd);

    return string(recv_buffer);
}

void proxy2client(int sockfd){
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
    in.open("proxy_cache.txt", ios::in);
    string first, second;
    while(in >> first >> second){
        if(first == "" || second == "") continue;
        cache.push_back({first, second});
        entry_cnt ++;
    }
    in.close();
    // debug(cache, entry_cnt);
    // Search if the request from client exists in the proxy cache
    // Type 1: Request for IP -> Search for mapping DOMAIN
    // Type 2: Request for Domain -> Search for mapping IP
    bool hit = false;
    string response;
    for(int i=0; i<entry_cnt; ++i){
        if(cache[i][type-1] == request){
            hit = true;
            response = "3" + cache[i][type%2];
            break;
        }
    }

    if(!hit){
        cout << "Miss in proxy cache.\n";
        response = proxy2dns(buffer);
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
        cout << "Proxy server is in LISTEN state.\n";
    }

    listen:
    int new_sockfd = 0, connection_cnt = 0;
    socklen_t proxy_addr_size = sizeof(proxy_addr);
    if((new_sockfd = accept(sockfd, (struct sockaddr*) &proxy_addr, &proxy_addr_size)) < 0){
        cout << "Accept failed!\n";
    }
    else {
        connection_cnt ++;
        cout << "Connection " << connection_cnt << " accepted from the client.\n";
    }

    // Function thread that takes care of further incoming connections
    int pid =  fork();
    if(pid == 0)
        proxy2client(new_sockfd);
    else
        goto listen;
    
    // Free socket
    close(sockfd);
    return 0;
}

