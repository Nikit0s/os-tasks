#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

typedef int bool;
#define true 1
#define false 0

long long int numbersLength = 0;

int comparator(const void* a, const void* b)
{
  if (*((long long int*)a) > *((long long int*)b)) return  1;
  if (*((long long int*)a) < *((long long int*)b)) return -1;
  return 0;
}

int main(int argc, char * argv[]) {
	if (argc < 3) {
		printf("Wrong arguments\n");
	}

	int i;

	long long int* numbers = (long long int*) malloc(sizeof(long long int));
	if (numbers == NULL) {
			printf("Cant allocate memory\n");
			exit(1);
	};

  // Opening files and looking for numbers
	for (i = 1; i < argc - 1; i++) {

    // Open file
    int fd = open(argv[i], O_RDONLY);
  	if (fd == -1) {
  		printf("Error openning file with numbers\n");
  		exit(1);
  	}

    // Initiate variables
  	char byte;
  	long long int number = 0;
  	bool readNumber = false;
  	int digit;
		bool isNegative = false;

    // Parse file
  	while (read(fd, &byte, 1) > 0) {
			if (byte >= '0' && byte <= '9') {
				digit = byte - '0';
				if (readNumber) {
  				number = number * 10 + digit;
  			} else {
  				readNumber = true;
  				number = digit;
  			}
			} else {
				if (readNumber) {
          if (isNegative) {
            number *= -1;
          }
					numbers = realloc(numbers, numbersLength * sizeof(long long int) + 1);
          if (numbers == NULL) {
            printf("Cant allocate memory\n");
            exit(1);
          };
					numbers[numbersLength] = number;

          numbersLength += 1;
  				readNumber = false;
  				number = 0;
					isNegative = false;
				} else {
					if (byte == '-') {
						isNegative = true;
						continue;
					} else {
            isNegative = false;
          }
				}
			}
  	}

    // If the last symbols were number
    if (readNumber) {
      numbers = realloc(numbers, numbersLength * sizeof(long long int) + 1);
      if (numbers == NULL) {
        printf("Error openning file with numbers\n");
        exit(1);
      };
      numbers[numbersLength] = number;
    }

		close(fd);
	}

	qsort(numbers, numbersLength, sizeof(long long int), comparator);

  int fd = open(argv[argc - 1], O_WRONLY | O_CREAT);
  if (fd == -1) {
    printf("Cant open file\n");
    exit(1);
  }

	for (i = 0; i < numbersLength; i++) {
		int res = dprintf(fd, "%lld \r\n", numbers[i]);
		if (res < 0) {
			printf("Cant write ti file");
			exit(1);
		}
	}

	close(fd);
}
