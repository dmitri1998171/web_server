#include <arpa/inet.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <thread>

#include "spdlog/spdlog.h"

#define PORT 8080

#define BUFFER_SIZE 1024
#define PARAMS_BUF 10
#define HEADER_SIZE 75

using namespace std;

char httpHead[HEADER_SIZE] = "HTTP/1.0 200 OK\r\nServer: webserver-c\r\nContent-type: text/html\r\n\r\n";

/* Read HTML and create responce message with HTTP-header */ 
void parseHTML(char msg[BUFFER_SIZE]) {
    char content[BUFFER_SIZE];

    FILE *fileHTML;
    if ((fileHTML = fopen("content/text.html", "r")) == NULL) {
        perror("Error: Cannot open file.\n");
        spdlog::error("CANNOT OPEN FILE!");
        exit(1);
    }

    // Add header to result message
    strcpy(msg, httpHead);

    // Reading the HTML & string concatenation
    while ( ! feof(fileHTML)) {
        fgets(content, BUFFER_SIZE, fileHTML);
        strcat(msg, content);
    }
    
    fclose(fileHTML);
}

void HTTPhandler(int newsockfd, char* ip, unsigned port) {
    char rcv_buffer[BUFFER_SIZE];
    char msg[BUFFER_SIZE];

    // Receive request from client
    int valread = read(newsockfd, rcv_buffer, BUFFER_SIZE);
    if (valread < 0) {
        cout << "THE SERVER COULD NOT RECEIVE REQUEST FROM CLIENT!" << endl;
        spdlog::error("THE SERVER COULD NOT RECEIVE REQUEST FROM CLIENT!");
    }

    // Read the request
    char method[PARAMS_BUF], uri[BUFFER_SIZE], version[PARAMS_BUF];
    sscanf(rcv_buffer, "%s %s %s", method, uri, version);
    rcv_buffer[strlen(rcv_buffer)] = '\0';

    parseHTML(msg);

    printf("[%s:%u] %s %s %s\n", ip, port, method, version, uri);
    printf("%s \n", msg);

    // Send response to client
    int valwrite = write(newsockfd, msg, strlen(httpHead));
    if (valwrite < 0) {
        spdlog::error("THE SERVER COULD NOT SEND RESPONSE!");
    }

    close(newsockfd);
}

int main() {
    spdlog::info("New accept to client");
    
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("webserver (socket)");
        spdlog::error("THE SERVER COULD NOT CREATE SOCKET!");
        exit(1);
    }
    printf("socket created successfully\n");

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
        perror("webserver (bind)");
        spdlog::error("THE SERVER COULD NOT DO BIND!");
        exit(1);
    }
    printf("socket successfully bound to address\n");

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {
        perror("webserver (listen)");
        spdlog::error("THE SERVER COULD NOT DO LISTEN!");
        exit(1);
    }
    printf("server listening for connections\n");

    for (;;) {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0) {
            perror("webserver (accept)");
            spdlog::error("THE SERVER COULD NOT DO ACCEPT!");
            continue;
        }
        printf("connection accepted\n");

        // Get client address
        if (getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen) < 0) {
            perror("webserver (getsockname)");
            spdlog::error("THE SERVER COULD NOT DO getsockname!");
            continue;
        }

        // Create thread
        thread (HTTPhandler, newsockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)).detach();
    }

    return 0;
}