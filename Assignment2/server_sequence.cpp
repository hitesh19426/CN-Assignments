// C program for the server Side

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include<netinet/in.h>
#include<netinet/ip.h>
#include <unistd.h>

#define MAX_FACTORIAL 20
#define PORT 8080
#define MAX_BACKLOG 10

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

// Driver Code
int main()
{
	int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketfd < 0){
        perror("socker creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    
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
    
    int64_t num = -1, fac = -1;
    int sz = sizeof(struct sockaddr_in);
    bool firstConnection = true;
    while(true){
        // sleep(5);
        int clientfd = accept(socketfd, (struct sockaddr*)&client, (socklen_t*)&sz);
        if(clientfd < 0){
            perror("accept error");
            exit(EXIT_FAILURE);
        }

        (firstConnection ? printf("connection accepted\n") : printf("\n\nconnection accepted\n"));
        printf("IP address is: %s\n", inet_ntoa(client.sin_addr));
        printf("port is: %d\n\n", (int) ntohs(client.sin_port));

        FILE *filePointer;
        filePointer = (firstConnection ? fopen("output_seq.txt", "w") : fopen("output_seq.txt", "a"));

        if(filePointer == NULL){
            perror("Unable to open file");
            exit(EXIT_FAILURE);
        }

        for(int i=0; i<20; i++){
            int r = recv(clientfd, &num, sizeof(num), 0);
            printf("Number from client is : %ld\n", num);

            int64_t fac = factorial(num);

            fprintf(filePointer, "client - %s, %d: The factorial for %ld is %ld \n", 
                inet_ntoa(client.sin_addr), ntohs(client.sin_port), num, fac);

            printf("Factorial of %ld is: %ld \n", num, fac);
            send(clientfd, &fac, sizeof(fac), 0);
        }

        firstConnection = false;
        fprintf(filePointer, "\n");
        fclose(filePointer);
        close(clientfd);
    }

    // fclose(filePointer);
    shutdown(socketfd, SHUT_RDWR);
	return EXIT_SUCCESS;
}
