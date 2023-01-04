// C program for the server Side

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define MAX_FACTORIAL 20
#define PORT 8080
#define MAX_BACKLOG 10
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

    if(listen(socketfd, MAX_BACKLOG) < 0){
        perror("listen failed");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    system("rm -f -- output_poll.txt");
    struct pollfd fds[200];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = socketfd;
    fds[0].events = POLLIN;

    int timeout = TIMEOUT_SECONDS*1000, nfds = 1;
    bool end_server = false, compress_array = false, close_connection = false;

    while(end_server == false){

        printf("waiting for poll()...");
        int res = poll(fds, nfds, -1);

        if(res < 0){
            perror("poll falied");
            break;
        }
        if(res == 0){
            perror("poll timed out");
            break;
        }

        int current_size = nfds, clientfd = 0;
        for(int i=0; i<current_size; i++){

            if(fds[i].revents == 0){
                continue;
            }
            if(fds[i].revents != POLLIN){
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = true;
                break;
            }
            if(fds[i].fd == socketfd){
                // printf("listening socket is readable\n");
                int sz = sizeof(struct sockaddr_in);
                while(clientfd != -1) {
                    clientfd = accept(socketfd, (sockaddr*)&client, (socklen_t*)&sz);
                    if (clientfd < 0) {
                        if(errno != EWOULDBLOCK) {
                            perror("accept() failed");
                            end_server = true;
                        }
                        break;
                    }

                    // printf("New incoming connection - %d\n", clientfd);
                    fds[nfds].fd = clientfd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                }
            }else{
                // printf("Descriptor %d is readable\n", fds[i].fd);
                close_connection = false;
                
                int64_t num;
                while(true){
                    int r = recv(fds[i].fd, &num, sizeof(num), 0);
                    // printf("Number from client is : %ld\n", num);

                    if(r < 0) {
                        if(errno != EWOULDBLOCK) {
                            perror("recv failed. <0 bytes received.");
                            close_connection = true;
                        }
                        break;
                    }
                    if(r == 0){
                        printf("connection closed \n");
                        close_connection = true;
                        break;
                    }

                    int64_t fac = factorial(num);

                    FILE *filePointer = fopen("output_poll.txt", "a");
                    fprintf(filePointer, "client - %s, %d: The factorial for %ld is %ld \n", 
                        inet_ntoa(client.sin_addr), ntohs(client.sin_port), num, fac);
                    fclose(filePointer);

                    // printf("Factorial of %ld is: %ld \n", num, fac);
                    if(send(fds[i].fd, &fac, sizeof(fac), 0) < 0){
                        perror("send() failed");
                        close_connection = true;
                        break;
                    }
                }
                // close_connection = true;
                if(close_connection){
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = true;
                }   
            }
        }

        if(compress_array){
            compress_array = false;
            for(int i=0; i<nfds; i++){
                if(fds[i].fd == -1){
                    for(int j=i; j<nfds; j++){
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--, nfds--;
                }
            }
        }

    }

    for(int i=0; i<nfds; i++){
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }

    shutdown(socketfd, SHUT_RDWR);
	return EXIT_SUCCESS;
}

void* handleConnection(int clientfd, struct sockaddr_in client){
    int64_t num = -1, fac = -1;
    for(int i=0; i<20; i++){
        FILE *filePointer;
        filePointer = fopen("select_output.txt", "a");

        if(filePointer == NULL){
            perror("Unable to open file");
            return NULL;
        }

        int r = recv(clientfd, &num, sizeof(num), 0);
        printf("Number from client is : %ld\n", num);

        int64_t fac = factorial(num);

        fprintf(filePointer, "client - %s, %d: The factorial for %ld is %ld \n", 
            inet_ntoa(client.sin_addr), ntohs(client.sin_port), num, fac);

        fclose(filePointer);

        printf("Factorial of %ld is: %ld \n", num, fac);
        send(clientfd, &fac, sizeof(fac), 0);
    }

    close(clientfd);
    return NULL;
}
