#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

char * getLockName(char * fileName) {
  char * lockName = malloc(strlen(fileName) + 4);
  strcpy(lockName, fileName);
  strcat(lockName, ".lck");
  return lockName;
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf("ERROR: Wrong arguments\n");
    return 1;
  };

  char *lockName;
  lockName = getLockName(argv[1]);

  // Wait while locking file exists
  int is_exist = -1;
  is_exist = access(lockName, 0);
  while(is_exist == 0) {
    is_exist = access(lockName, 0);
  };

  int s;

  // Create locking file
  FILE *fp = fopen(lockName, "wa");
  if (fp) {
    pid_t pid = getpid();
    s = fprintf(fp, "%d:write", pid);
    if (s < 0) {
      printf("ERROR: Cant write to locking file\n");
      return 1;
    };
    fclose(fp);
  };

  // Edit file
  fp = fopen(argv[1], "a");
  if (fp == NULL) {
    printf("ERROR: Cant open editing file\n");
    return 1;
  };

  s = fprintf(fp, "%s\n", argv[2]);
  if (s < 0) {
    printf("ERROR: Cant write to editing file\n");
    return 1;
  };

  fclose(fp);

  // Delete locking file
  s = remove(lockName);

  return 0;
};
