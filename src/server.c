#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8080

#define BUFFER_SIZE 1024
#define PARAMS_BUF 10

int main() {
    char buffer[BUFFER_SIZE];
    char httpHead[BUFFER_SIZE] = "HTTP/1.0 200 OK\r\nServer: webserver-c\r\nContent-type: text/html\r\n\r\n";
    char content[BUFFER_SIZE];

    FILE *fileHTML;
    if ((fileHTML = fopen("text.html", "r")) == NULL) {
        perror("Error: Cannot open file.\n");
        exit(1);
    }

    // String concatenation
    while ( ! feof(fileHTML)) {
        fgets(content, BUFFER_SIZE, fileHTML);
        strcat(httpHead, content);
    }
    
    fclose(fileHTML);

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("webserver (socket)");
        return 1;
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
        return 1;
    }
    printf("socket successfully bound to address\n");

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {
        perror("webserver (listen)");
        return 1;
    }
    printf("server listening for connections\n");

    for (;;) {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        // Get client address
        int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getsockname)");
            continue;
        }

        // Recieve request from client
        int valread = read(newsockfd, buffer, BUFFER_SIZE);
        if (valread < 0) {
            perror("webserver (read)");
            continue;
        }

        // Read the request
        char method[PARAMS_BUF], uri[BUFFER_SIZE], version[PARAMS_BUF];
        sscanf(buffer, "%s %s %s", method, uri, version);
        buffer[strlen(buffer)] = '\0';

        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, uri, version);
        printf("%s \n",buffer);

        // Send response to client
        int valwrite = write(newsockfd, httpHead, strlen(httpHead));
        if (valwrite < 0) {
            perror("webserver (write)");
            continue;
        }

        close(newsockfd);
    }

    return 0;
}