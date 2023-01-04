// C program for the server Side
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_FACTORIAL 20
#define PORT 8080
#define MAX_BACKLOG 10

void* handleConnection(void* clientfd);

int64_t factorial(int64_t n){
    if(n < 0)
        return -1;
    if(n <= 1)
        return 1;
    int64_t ans = 1;
    for(int i=1; i<= n; i++)
        ans *= i;
    return ans;
}

struct client_args{
    int clientfd;
    struct sockaddr_in client;
};

// Driver Code
int main()
{
	int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketfd == -1){
        perror("socker creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;    // internet socket
    server.sin_port = htons(PORT);          // htons converts
    server.sin_addr.s_addr = INADDR_ANY;    // connect to any internet address
    
    int opt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if(bind(socketfd, (struct sockaddr*) &server, sizeof(server)) < 0){
        perror("Binding error");
        exit(EXIT_FAILURE);
    }

    if(listen(socketfd, MAX_BACKLOG) < 0){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    
    system("rm -f -- output_pthread.txt");
    while(true){
    
        printf("Waiting for connections ... \n");
        int sz = sizeof(struct sockaddr_in);

        int clientfd = accept(socketfd, (struct sockaddr*)&client, (socklen_t*)&sz);
        if(clientfd == -1){
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        // printf("\nconnection accepted\n");
        // printf("IP address is: %s\n", inet_ntoa(client.sin_addr));
        // printf("port is: %d\n\n", (int) ntohs(client.sin_port));

        pthread_t thrd;
        struct client_args *args = (struct client_args*)malloc(sizeof(client_args));
        args->client = client;
        args->clientfd = clientfd;
        pthread_create(&thrd, NULL, handleConnection, args);
    }

    // fclose(filePointer);
    shutdown(socketfd, SHUT_RDWR);
	return EXIT_SUCCESS;
}

void* handleConnection(void* p_args){
    struct client_args args = *((struct client_args*)p_args);
    free(p_args);
     
    int clientfd = args.clientfd;
    struct sockaddr_in client = args.client;

    int64_t num = -1, fac = -1;
    for(int i=0; i<20; i++){
        // FILE *filePointer = fopen("output_pthread.txt", "a");
        // if(filePointer == NULL){
        //     perror("Unable to open file");
        //     close(clientfd);
        //     exit(EXIT_FAILURE);
        // }

        int r = recv(clientfd, &num, sizeof(num), 0);
        // printf("Number from client is : %ld\n", num);

        int64_t fac = factorial(num);

        // fprintf(filePointer, "client - %s, %d: The factorial for %ld is %ld \n", 
        //     inet_ntoa(client.sin_addr), ntohs(client.sin_port), num, fac);
        // fclose(filePointer);

        // printf("Factorial of %ld is: %ld \n", num, fac);
        send(clientfd, &fac, sizeof(fac), 0);
    }

    close(clientfd);
    return NULL;
}
