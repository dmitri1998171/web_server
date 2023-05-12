#include <arpa/inet.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include "spdlog/spdlog.h"


#define PORT 8080

#define BUFFER_SIZE 1024
#define PARAMS_BUF 10
#define HEADER_SIZE 75

using namespace std;

mutex mtx;
char httpHead[HEADER_SIZE] = "HTTP/1.0 200 OK\r\nServer: webserver-c\r\nContent-type: text/html\r\n\r\n";

/* Read HTML and create responce message with HTTP-header */ 
void parseHTML(char msg[BUFFER_SIZE]) {
    char content[BUFFER_SIZE];

    FILE *fileHTML;
    if ((fileHTML = fopen("content/text.html", "r")) == NULL) {
        spdlog::error("CANNOT OPEN FILE!");
        exit(1);
    }

    // Add header to result message
    mtx.lock();
    strncpy(msg, httpHead, HEADER_SIZE);
    mtx.unlock();

    // Reading the HTML & string concatenation
    while ( ! feof(fileHTML)) {
        fgets(content, BUFFER_SIZE, fileHTML);
        strcat(msg, content);
    }
    
    fclose(fileHTML);
}

void HTTPhandler(int newsockfd, char msg[BUFFER_SIZE]) {
    char rcv_buffer[BUFFER_SIZE];
    char method[PARAMS_BUF], uri[BUFFER_SIZE], version[PARAMS_BUF];

    // Receive request from client
    if (read(newsockfd, rcv_buffer, BUFFER_SIZE) < 0) {
        spdlog::error("THE SERVER COULD NOT RECEIVE REQUEST FROM CLIENT!");
    }

    // Read the request
    sscanf(rcv_buffer, "%s %s %s", method, uri, version);
    rcv_buffer[strlen(rcv_buffer)] = '\0';

    if( (strcmp(uri, "/" ) != 0) && (strcmp(uri, "#send") != 0) ){
        spdlog::info("uri: {}", uri);
        spdlog::error("Error: 404 page does not exist");
        exit(1);
    }

    cout << "\n==================================================\n\n";
    cout << "[*] REQUEST HEADER: " << endl << rcv_buffer << endl << endl;
    cout << "[*] RESPONCE HEADER: " << endl << msg << endl << endl;

    // Send response to client
    if (write(newsockfd, msg, strlen(msg)) < 0) {
        spdlog::error("THE SERVER COULD NOT SEND RESPONSE!");
    }

    close(newsockfd);
}

int main() {
    spdlog::info("Server started");
    
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        spdlog::error("THE SERVER COULD NOT CREATE SOCKET!");
        exit(1);
    }
    cout << "socket created successfully\n";

    // Allows you to run multiple instances of the same server on the same port
    const int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
    	spdlog::error("Setsockopt(SO_REUSEADDR) failed. \n");
      exit(1);
    }

    // Create the address to bind the socket to
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create client address
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        spdlog::error("THE SERVER COULD NOT DO BIND!");
        exit(1);
    }
    cout << "socket successfully bound to address\n";

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {
        spdlog::error("THE SERVER COULD NOT DO LISTEN!");
        exit(1);
    }
    cout << "server listening for connections\n";

    char msg[BUFFER_SIZE];
    parseHTML(msg);

    for (;;) {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0) {
            spdlog::error("THE SERVER COULD NOT DO ACCEPT!");
            continue;
        }
        cout << "connection accepted\n";
        spdlog::info("New accept to client");

        // Get client address
        if (getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen) < 0) {
            spdlog::error("THE SERVER COULD NOT DO getsockname!");
            continue;
        }

        // Create thread
        thread (HTTPhandler, newsockfd, msg).detach();
    }

    return 0;
}