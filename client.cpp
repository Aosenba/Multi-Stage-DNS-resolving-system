#include <iostream>
#include <sys/socket.h> // to get the socket api
#include <arpa/inet.h> // makes in_addr available and in_addr_t in_port_t
#include <unistd.h> // makes the posix api available to be used
#include <string>

using namespace std;
int main(int argc, char const *argv[]){
    string client_ipaddr = string(argv[1]);
    int client_port = stoi(string(argv[2]));
    int sockfd = 0; // socket file discriptor


    // Setting up the server address
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;

    // setting the server ip address to the ip address provided by
    // first converting the ip address obtained to binary from the dot
    // seperated format. This is done by inet_addr()
    client_addr.sin_addr.s_addr = inet_addr(client_ipaddr.c_str());
    
    // htons() makes sure that the port number is in network byte order (big endian format)
    // irrespective of the format in which the number is saved in memory (little endian format)
    client_addr.sin_port = htons(client_port);

    // Creating the socket file discriptor
    // AF_INET: means using IPv4. For IPv6 use AF_INET6.
    // SOCK_STREAM: means using TCP. For UDP use SOCK_DGRAM.
    // 0: means using Internet Protocol.
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else{
        cout << "Socket is created succesfully\n";
    }

    // Establishing connection
    if(connect(sockfd, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
        cout << "Error in establishing the connection\n";
        exit(0);
    }
    else{
        cout << "Connected!\n";
    }

    // Send DNS request message
    // Get the message
    string request, type;
    cout << "<REQUEST TYPE>\n";
    cin >> request >> type;
    request = !request.find("www.") ? type + request.substr(4) : type + request;
    request = request.length() < 10 ? "0" + to_string(request.length()) + request : to_string(request.length()) + request;

    // Sending the message
    int bytes_send;
    if((bytes_send = send(sockfd, request.c_str(), request.length()*sizeof(char), 0)) < 0){
        cout << "Failed to send\n";
        exit(0);
    }
    else{
        cout << bytes_send << "B are transmitted successfully!\n";
    }

    // Recieving the response
    char response[256];
    int bytes_recv;
    if((bytes_recv = recv(sockfd, response, sizeof(response), 0)) < 0){
        cout << "Error!\n";
        exit(0);
    }
    else{
        cout << "The requested address: " << response + 1 << "\n";
    }

    // Free the socket
    close(sockfd);

    return 0;
}
