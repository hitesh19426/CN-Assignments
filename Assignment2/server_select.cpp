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

void* handleConnection(int, struct sockaddr_in);

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
    if(socketfd < 0){
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

    system("rm -f -- output_select.txt");
    int sz = sizeof(struct sockaddr_in);
    int max_socket_so_far = 0;
    fd_set current_sockets, ready_sockets;

    // initialize my current set
    FD_ZERO(&current_sockets);
    FD_SET(socketfd, &current_sockets);
    max_socket_so_far = socketfd;

    // printf("FD_SETSIZE = %d", FD_SETSIZE);
    // cant have more than FD_SETSIZE connections

    while(true){
        printf("Waiting for connections ... \n");

        // creating a copy of current sockets because select will modify current sockets
        ready_sockets = current_sockets;
        if(select(max_socket_so_far+1, &ready_sockets, NULL, NULL, NULL) < 0){
            perror("select error");
            exit(EXIT_FAILURE);
        }

        for(int i=0; i<max_socket_so_far+1; i++){
            if(FD_ISSET(i, &ready_sockets)){
                if(i == socketfd){
                    int clientfd = accept(socketfd, (struct sockaddr*)&client, (socklen_t*)&sz);
                    if(clientfd < 0){
                        perror("accept error");
                        exit(EXIT_FAILURE);
                    }

                    // printf("\nconnection accepted\n");
                    // printf("IP address is: %s\n", inet_ntoa(client.sin_addr));
                    // printf("port is: %d\n\n", (int) ntohs(client.sin_port));

                    FD_SET(clientfd, &current_sockets);
                    if(clientfd > max_socket_so_far){
                        max_socket_so_far = clientfd;
                    }
                }else{
                    
                    handleConnection(i, client);
                    FD_CLR(i, &current_sockets);
                }
            }
        }
    }

    // fclose(filePointer);
    shutdown(socketfd, SHUT_RDWR);
	return EXIT_SUCCESS;
}

void* handleConnection(int clientfd, struct sockaddr_in client){
    int64_t num = -1, fac = -1;
    for(int i=0; i<20; i++){
        // FILE *filePointer;
        // filePointer = fopen("output_select.txt", "a");

        // if(filePointer == NULL){
        //     perror("Unable to open file");
        //     return NULL;
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
