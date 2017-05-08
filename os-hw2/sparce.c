#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int sparce(int fd) {
	char buffer[100];
	int bytesRead = 0;
	int zeroCount = 0;

	while(bytesRead = read(0, buffer, 100)) {
		int i = 0;

		while (i < bytesRead) {
      zeroCount = 0;

			while(buffer[i] != 0) {
				write(fd, &buffer[i++], 1);
			}

			while(buffer[i] == 0 && i < bytesRead) {
				zeroCount++;
				i++;
			}
			if (zeroCount > 0) {
				lseek(fd, zeroCount, SEEK_CUR);
			}
		}

	}

	return 0;
}

int main(int argc, char * argv[]) {
  if (argc != 2) {
    printf("Wrong arguments count");
  }

	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

	if (fd == -1) {
		printf("Error with file");
		return -1;
	}

	sparce(fd);
  close(fd);
}
