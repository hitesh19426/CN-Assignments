#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_FACTORIAL 20
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

int main(){

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

	if(bind(socketfd, (struct sockaddr*)&server, sizeof(server)) < 0){
		perror("Binding error");
        exit(EXIT_FAILURE);
	}

	if(listen(socketfd, MAX_BACKLOG) < 0){
		perror("listen failed");
        exit(EXIT_FAILURE);
	}

	int64_t num = -1, fac = -1;
	int addr_size = sizeof(struct sockaddr_in);

	system("rm -f -- output_fork.txt");
	while(true){
		int clientfd = accept(socketfd, (struct sockaddr*)&client, (socklen_t*)&addr_size);
		if(clientfd < 0){
			perror("accept error");
            exit(EXIT_FAILURE);
		}
		// printf("\nconnection accepted\n");
        // printf("IP address is: %s\n", inet_ntoa(client.sin_addr));
        // printf("port is: %d\n\n", (int) ntohs(client.sin_port));

		int childpid = fork();
		if(childpid == 0){
			for(int i=1; i<=MAX_FACTORIAL; i++){
		        int r = recv(clientfd, &num, sizeof(num), 0);
		        // printf("Number from client is : %ld\n", num);

		        int64_t fac = factorial(num);

		        // FILE *filePointer = fopen("output_fork.txt", "a");
		        // fprintf(filePointer, "client - %s, %d: The factorial for %ld is %ld \n", 
                // 	inet_ntoa(client.sin_addr), ntohs(client.sin_port), num, fac);
		        // fclose(filePointer);

				// printf("Factorial of %ld is: %ld \n", num, fac);
		        send(clientfd, &fac, sizeof(fac), 0);
		    }

			close(clientfd);
		}

	}

	shutdown(socketfd, SHUT_RDWR);
	return EXIT_SUCCESS;
}