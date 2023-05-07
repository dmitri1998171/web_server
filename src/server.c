#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 56000
#define MSG_SIZE 1024

char * filename = "/Users/user/Programming/HTTP-server/text.html";
char * requestStatus = "HTTP/1.1 200 OK\r\n";

struct conectClient {
	int newSocket;
	pthread_t thread;
};

struct sockaddr_in serverAddr, newAddr;
typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = { {"\0", "\0"} };

// get request header
char *request_header(const char* name)
{
    int i = 0;
    header_t *h = reqhdr;
    while(h->name) {
        if (strcmp(h->name, name) == 0) return h->value;
        h++;
    }
    return NULL;
}

void respond(int socket)
{
    int rcvd, fd, bytes_read;
    char *ptr;

    char* buf = malloc(65535);
    rcvd=recv(socket , buf, 65535, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else    // message received
    {
        buf[rcvd] = '\0';

        char* method = strtok(buf,  " \t\r\n");
        char* uri    = strtok(NULL, " \t");
     	char* prot   = strtok(NULL, " \t\r\n"); 
		char* qs;

        fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);
        
        if (qs = strchr(uri, '?'))
        {
            *qs++ = '\0'; //split URI
        } else {
            qs = uri - 1; //use an empty string
        }

        header_t *h = reqhdr;
        char *t, *t2;
        while(h < reqhdr+16) {
            char *k,*v,*t;
            k = strtok(NULL, "\r\n: \t"); 
            if (!k) break;
            v = strtok(NULL, "\r\n");     
            while(*v && *v==' ') v++;
            h->name  = k;
            h->value = v;
            h++;
            fprintf(stderr, "[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n') break;
        }
        t++; // now the *t shall be the beginning of user payload
        t2 = request_header("Content-Length"); // and the related header if there is  
        char* payload = t;
        int payload_size = t2 ? atol(t2) : (rcvd-(t-buf));

		char* sendd = "HTTP/1.1 200 OK\r\n\r\n Hello! You are using ";
        // call router
        send(socket, sendd, strlen(sendd), 0);

		// printf("HTTP/1.1 200 OK\r\n\r\n");
        // printf("Hello! You are using %s", request_header("User-Agent"));
	}
    close(socket);
}


// _________________________________________

void *newSock(void *arg) {
	struct conectClient *links = (struct conectClient*) arg;
	
	pthread_detach(links->thread);

	respond(links->newSocket);
	request_header("User-Agent");

	// send(links->newSocket, requestStatus , strlen(buffer), 0);
	
	// FILE *fp = fopen(filename, "r");
	// if (fp == 0)
	// {
	// 	printf("ERROR! hmtl not found!");
	// 	exit(1);
	// }
		
	// int bytes_read = 0;
	// while( (fgets(buffer, 256, fp)) != NULL ) {
	// 	printf("%s, bytes_read: \n", buffer );
	// }
	// fclose(fp);

	// send(links->newSocket, buffer , strlen(buffer), 0);
	
	// }

	printf("[+]Closing the connection.\n");
	return NULL;
}

int main(){
	int sockfd;
	socklen_t structLength;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erorr: socket creation failed. \n");
        exit(1);
    }

	const int enable = 1;
	// Позволяет выставить флаги для сокета SO_REUSEADDR позволяет множеству экземпляров одного и того же сервера запускаться на одном и том же порте
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
    	printf("Setsockopt(SO_REUSEADDR) failed. \n");
		exit(1);
	}
    
	// Заполняет нулями
	bzero(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(PORT);

	// Связывает сокет с локальным адресом протокола
	if(bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		printf("Erorr: bind failed. \n");
		exit(1);
	}

	// Принимать входящие соединения и задет размер очереди
	if(listen(sockfd, 5) < 0) {
		printf("Erorr: listen failed. \n");
		exit(1);
	}
	
	structLength = sizeof(newAddr);

	while (1) {
		//	Динамическая стркутура через malloc
		struct conectClient *links = malloc(sizeof(struct conectClient));
		
		links->newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &structLength);
	
		if(pthread_create(&links->thread, NULL, *newSock, (void*) links) < 0){
			printf("Erorr: thread create failed. \n");
			exit(1);
		}
	}
	
	return 0;
}