// C program for the server Side
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define MAX_FACTORIAL 20
#define PORT 8080
#define MAX_CONNECTIONS 10
#define TIMEOUT_SECONDS 30

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

// Driver Code
int main()
{
	int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketfd < 0){
        perror("socker creation failed");
        exit(EXIT_FAILURE);
    }
    // printf("socket created\n");
    
    int opt = 1;
    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsocket() failed");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    
    // make a server non-blocking
    if(ioctl(socketfd, FIONBIO, (char*) &opt) < 0){
        perror("ioctl() failed");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;    // internet socket
    server.sin_port = htons(PORT);          // htons converts
    server.sin_addr.s_addr = INADDR_ANY;    // connect to any internet address

    if(bind(socketfd, (struct sockaddr*) &server, sizeof(server)) < 0){
        perror("Binding error");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    if(listen(socketfd, MAX_CONNECTIONS) < 0){
        perror("listen failed");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    system("rm -f -- output_epoll.txt");
    // struct pollfd *pdfs;
    struct epoll_event pollfd, pollfds[MAX_CONNECTIONS+1];
    memset(pollfds, 0, sizeof(pollfds));

    int efd = epoll_create1(0);
    if(efd < 0){
        perror("epoll create1 error");
        exit(EXIT_FAILURE);
    }

    pollfd.data.fd = socketfd;
    pollfd.events = EPOLLIN;
    
    if(epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &pollfd) < 0){
        perror("epoll_ctl error");
        exit(EXIT_FAILURE);
    }
    // printf("connection setup complete.\n");

    while(true){
        
        // printf("waiting for epoll...\n");
        int nfds = epoll_wait(efd, pollfds, MAX_CONNECTIONS, -1);
        // printf("nfds = %d \n", nfds);

        if(nfds < 0){
            perror("epoll wait error");
            break;
        }
        if(nfds == 0){
            perror("epoll timed out");
            break;
        }
        
        int sz = sizeof(struct sockaddr_in);
        for(int i=0; i<nfds; i++){
            if(pollfds[i].data.fd == socketfd){
                struct sockaddr_in client_address;
                int clientfd = accept(socketfd, (struct sockaddr*)&client, (socklen_t*)&sz);
                if(clientfd < 0){
                    perror("accept error");
                    exit(EXIT_FAILURE);
                }
                // printf("accepted connection from %d \n", clientfd);

                pollfd.events = EPOLLIN;
                pollfd.data.fd = clientfd;
                if(epoll_ctl(efd, EPOLL_CTL_ADD, clientfd, &pollfd) < 0){
                    perror("epoll_ctl error");
                    exit(EXIT_FAILURE);
                }   
            }
            else{
                int64_t num;
                while(true){
                    int r = recv(pollfds[i].data.fd, &num, sizeof(num), 0);
                    // printf("Number from client is : %ld\n", num);

                    if(r < 0) {
                        perror("recv failed.");
                        break;
                    }
                    else if(r == 0){
                        break;
                    }else{
                        int64_t fac = factorial(num);

                        FILE *filePointer = fopen("output_epoll.txt", "a");
                        fprintf(filePointer, "client - %s, %d: The factorial for %ld is %ld \n", 
                            inet_ntoa(client.sin_addr), ntohs(client.sin_port), num, fac);
                        fclose(filePointer);

                        // printf("Factorial of %ld is: %ld \n", num, fac);
                        if(send(pollfds[i].data.fd, &fac, sizeof(fac), 0) < 0){
                            perror("send() failed");
                            break;
                        }
                    }
                }
                close(pollfds[i].data.fd);
                // printf("closed the connection for %d\n", pollfds[i].data.fd);
            }
        }
    }

    for(int i=0; i<MAX_CONNECTIONS; i++){
        close(pollfds[i].data.fd);
    }
    // free(pollfds);
    shutdown(socketfd, SHUT_RDWR);
	return EXIT_SUCCESS;
}
