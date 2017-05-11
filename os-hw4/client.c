#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#define HEIGHT 10
#define WIDTH 10

int field[HEIGHT][WIDTH];

void printField() {
	int i, j;
	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < HEIGHT; j++) {
			printf("%d ", field[i][j]);
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	int sockfd, n;
	int portno = 5555;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		error("ERROR: Cant create socket\n");
	}

	// Hostname to ip
	server = gethostbyname("localhost");

	if (server == NULL) {
		error("ERROR: Cant find such host\n");
	}

	// Set socket type
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	// Set ip adress of socket
	bcopy((char *) server -> h_addr, (char *) &serv_addr.sin_addr.s_addr, server -> h_length);

	// Set socket port
	serv_addr.sin_port = htons(portno);

	// Set connection
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR: Cant connect to server\n");
	 }

	//  Read message
	 n = read(sockfd, field, HEIGHT * WIDTH * sizeof(int));
	 if (n < 0) {
	 		error("ERROR: Reading from socket");
	 }

	 printField();

	 return 0;
}
