#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_CLIENT 10
#define PORT 8080
#define MAX_FACTORIAL 20
void *connection_handler(void *);

int main()
{
    pthread_t sniffer_thread[NUM_CLIENT];
	int id;
    for (int i=0; i<NUM_CLIENT; i++) {
        pthread_create(&sniffer_thread[i], NULL ,  connection_handler , NULL);
    }
    for(int i=0; i<NUM_CLIENT; i++){
        pthread_join(sniffer_thread[i], NULL);
    }
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}

void *connection_handler(void *threadid)
{
    // int threadnum = *((int*)threadid);
    int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketfd < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
	}

	struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
	// server.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0){
		printf("\n Invalid address");
		exit(EXIT_FAILURE);
	}

	int clientfd = connect(socketfd, (struct sockaddr*)&server, sizeof(server));
    if(clientfd < 0){
        perror("Failed to connect to server\n");
		exit(EXIT_FAILURE);
    }

    // printf("connected to server\n");
    int64_t num_to_pass = 1, ans = -1;
    for(int i=1; i<=MAX_FACTORIAL; i++){
        num_to_pass = i;
        // printf("passing %ld to server \n", num_to_pass);
        send(socketfd, &num_to_pass, sizeof(num_to_pass), 0);
        
        int bitrecv = recv(socketfd, &ans, sizeof(ans), 0);
        // printf("Factorial of %ld recd from server is %ld: \n", num_to_pass, ans);
    }
    close(socketfd);
    return NULL;
}