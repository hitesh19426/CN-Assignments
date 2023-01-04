// C program for the client Side

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#define MAX_FACTORIAL 20
#define PORT 8080

// Driver Code
int main()
{
	int socketid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketid == -1){
        perror("socket creation failed");
        exit(0);
    }

    struct sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
		printf( "\nInvalid address/ Address not supported \n");
		return -1;
	}
    
    int clientfd = connect(socketid, (struct sockaddr*)&server, sizeof(server));
    if(clientfd == -1){
        perror("\nConnection Failed \n");
        exit(0);
    }

    printf("connected to server\n");
    // sleep(5);
    int64_t num_to_pass = 1, ans = -1;
    for(int i=1; i<=MAX_FACTORIAL; i++){
        num_to_pass = i;
        printf("passing %ld to server \n", num_to_pass);
        send(socketid, &num_to_pass, sizeof(num_to_pass), 0);
        
        int bitrecv = recv(socketid, &ans, sizeof(ans), 0);
        printf("Factorial of %ld recd from server is %ld: \n", num_to_pass, ans);
    }
    // num_to_pass = -1;
    // send(socketid, &num_to_pass, sizeof(num_to_pass), 0);

    close(clientfd);
	return 0;
}
