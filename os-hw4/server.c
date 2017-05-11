#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

#define HEIGHT 10
#define WIDTH 10
#define FIELD_FILE "field.txt"

int field[HEIGHT][WIDTH];
int tempField[HEIGHT][WIDTH];

void readPointNeighbours(int nb[][2], int x, int y) {
	int i, j;
	int k = 0;

	for (i = x - 1; i <= x + 1; i++) {
		for (j = y - 1; j <= y + 1; j++) {
			if (i == x && j == y) {
				continue;
			}
			nb[k][0] = i;
			nb[k][1] = j;
			k++;
		}
	}
}

int countNeighbours(int x, int y) {
	int count = 0;
	int i;
	int nb[8][2];
	int _x, _y;

	readPointNeighbours(nb, x, y);

	for (i = 0; i < 8; i++) {
		_x = nb[i][0];
		_y = nb[i][1];

		if (_x < 0 || _y < 0) {
			continue;
		}

		if (_x >= WIDTH || _y >= HEIGHT) {
			continue;
		}

		if (field[_x][_y] == 1) {
			count++;
		}
	}

	return count;
}

void tempToCurrentField() {
	int i, j;
	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < HEIGHT; j++) {
			field[i][j] = tempField[i][j];
		}
	}
}

int lifeStep() {
	int i, j, liveCellsCount;
	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < HEIGHT; j++) {
			liveCellsCount = countNeighbours(i, j);
			switch(liveCellsCount) {
				case 2:
					tempField[i][j] = field[i][j];
					break;
				case 3:
					tempField[i][j] = 1;
					break;
				default:
					tempField[i][j] = 0;
			}
		}
	}
	tempToCurrentField();
}

void sendField(int sockfd) {
	if (write(sockfd, field, HEIGHT * WIDTH * sizeof(int)) < 0) {
		error("ERROR: Cant write message to client socket\n");
		return;
	}
}

void serverRoutine() {
	int sockfd, client_sockfd, clilen;
	int portno = 5555;
  struct sockaddr_in serv_addr, client_addr;
	int pid;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    error("ERROR: opening socket");
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr , sizeof(serv_addr)) == -1) {
    error("ERROR: Cant bind socket\n");
  }

  listen(sockfd, 10);
	clilen = sizeof(client_addr);

  printf("Server listening on %d!\n", portno);

  while (1) {
  	client_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clilen);
    if (client_sockfd < 0) {
      error("ERROR: Cant accept client\n");
    }

    // Multiply client work
	  pid = fork();

		if (pid < 0) {
			printf("Error with fork\n");
			return;
		}

		if (pid == 0) {
			close(sockfd);
			sendField(client_sockfd);
			exit(0);
		} else {
			close(client_sockfd);
		}
  }
}

int main(int argc, char *argv[]) {

	// Read field info
	FILE *fd = fopen(FIELD_FILE, "r");
  if (fd == NULL) {
    error("ERROR: Cant open file\n");
  };

	int symb;
	int i = 0;
	int j = 0;

	while (i < HEIGHT) {
		j = 0;
    while (j < WIDTH) {
			symb = fgetc(fd);
			switch(symb) {
        case '1':
            field[i][j] = 1;
						j++;
            break;
        case '0':
            field[i][j] = 0;
						j++;
            break;
				case EOF:
					error("Unexpected EOF\n");
					break;
        default:
            break;
    	};
    };

    i += 1;
  };

	fclose(fd);

	pthread_t serverThread;
	int status;

	// Starting server thread
	if (status = pthread_create(&serverThread, NULL, &serverRoutine, NULL)) {
		printf("Can't create server thread, status = %d\n", status);
    exit(1);
	}

	// 1 sec threads
	pthread_t lifeThread;
	while(1) {

		if (status = pthread_create(&lifeThread, NULL, &lifeStep, NULL)) {
			printf("Can't create life thread, status = %d\n", status);
			exit(1);
		}

		// Sleep 1 second
		sleep(1);

		// Error if more than 1 second
		if (pthread_kill(lifeThread, 0) != ESRCH) {
			pthread_cancel(lifeThread);
			printf("Error: Thread took more than 1 second\n");
			exit(1);
		}
	}

	return 0;
}
