#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>

#define LINE_SIZE 200
#define ARG_LENGTH 100
#define MAX_PROCS 100
#define SLEEP_TIME_SECS 5

int gargc;
char ** gargv;

typedef int bool;
#define true 1
#define false 0

pid_t pids[MAX_PROCS];
int pids_count;

int programsCount;

struct program {
  char name[100];
  char *arguments[20];
  int mode;
  int tries;
};

struct program programs[MAX_PROCS];

void setMode(int argc, int mode) {
  programs[programsCount].arguments[argc] = NULL;
  programs[programsCount].mode = mode;
}

int parseConfig(char *cfg) {
  FILE *fd;
  if ((fd = fopen(cfg, "r")) == NULL) {
    printf("ERROR: Can't open config\n");
    return 1;
  }

  char *line = malloc(LINE_SIZE);
  char *temp;

  while (fgets(line, LINE_SIZE, fd) != NULL)  {
      temp = strtok(line,  ":");
      strcpy(programs[programsCount].name, temp);

      programs[programsCount].arguments[0] = (char*) malloc(ARG_LENGTH);
      strcpy(programs[programsCount].arguments[0], temp);
      int argc = 1;

      while ((temp = strtok(NULL, ":")) != NULL)  {

        if (strcmp(temp, "wait") == 0) {
          setMode(argc, 0);
          break;
        }

        if (strcmp(temp, "respawn") == 0) {
          setMode(argc, 1);
          break;
        }

        programs[programsCount].arguments[argc] = (char*) malloc(ARG_LENGTH);
        strcpy(programs[programsCount].arguments[argc], temp);
        argc++;
      }

      programsCount++;
  }

  // Initialize launh tries array
  int j;
  for (j = 0; j < programsCount; j++) {
    programs[j].tries = 0;
  }

  fclose(fd);
  return 0;
}

int saveToFile(pid_t pid, int i) {
  char *name = programs[i].name;
  if (name[0] != '.') {
    name = basename(name);
  }

  char filePath[300];
  snprintf(filePath, sizeof(filePath), "/tmp/%s_%d.pid", name, i);

  FILE *fd;
  if ((fd = fopen(filePath, "w")) == NULL) {
    printf("ERROR: Can't open file %s\n", filePath);
    return 1;
  }
  fwrite(&pid, sizeof(pid_t), 1, fd);
  fclose(fd);
  return 0;
}

// i - process number
int makeFork(int i, bool is_sleep) {
  // pid_t pid = fork();
  int pid;

  switch (pids[i] = fork()) {
    // ERROR handler
    case -1:
      // error("Fork failed");
      exit(1);
      break;

    case 0:
      if (is_sleep) {
        sleep(SLEEP_TIME_SECS);
      }

      if (execvp(programs[i].name, programs[i].arguments) < 0) {
        // error("Cant execute");
        exit(1);
      }

      exit(0);
      break;

    default:
      // pid = getpid();
      // printf("%d\n", pid);
      // pids[i] = pid;
      pids_count++;
      saveToFile(pids[i], i);
      break;
    }

    return 0;
}

void removeFile(int i) {
  char *name = programs[i].name;
  if (name[0] != '.') {
    name = basename(name);
  }

  char filePath[300];
  snprintf(filePath, sizeof(filePath), "/tmp/%s_%d.pid", name, i);
  remove(filePath);
}

int handleProcesses() {
  pid_t pid;
  int i;
  int status;
  while (pids_count) {
    pid = wait(&status);
    // pid = waitpid(WAIT_ANY, NULL, 0);
    // Find any done pid
    for (i = 0; i < programsCount; i++) {
      if (pids[i] != pid) {
        continue;
      }

      if (programs[i].mode == 0) {
        // if mode - WAIT
        int j = 0;
        while (programs[i].arguments[j] != NULL) {
          free(programs[i].arguments[j]);
          j++;
        }

        removeFile(i);

        pids[i] = 0;
        pids_count--;

      } else {
        // if mode - respawn

        if (status != 0) {
          programs[i].tries++;
        }
        if (programs[i].tries > 50) {
          printf("%s program execution failed more than 50 times. Taking pause for %d seconds\n", programs[i].name, SLEEP_TIME_SECS);
          makeFork(i, true);
          programs[i].tries = 0;
        } else {
          makeFork(i, false);
        }

        // int err = makeFork(i);
        // if (err) {
        //   printf("ERROR: Can't fork!\n");
        //   return 1;
        // }
      }
    }
  }
  return 0;
}

void hupHandler(int sig) {
  int i;
  for (i = 0; i < programsCount; i++) {
    if (pids[i] > 0) {
      kill(pids[i], SIGKILL);
    };
  };

  run(gargc, gargv);
}

int run(int argc, char * argv []) {

  pids_count = 0;

  programsCount = 0;


  if (argc < 2) {
    printf("ERROR: Wrong arguments\n");
    return 1;
  }

  int err = parseConfig(argv[1]);
  if (err) {
    return 1;
  }

  int i;

  for (i = 0; i < programsCount; i++) {
    err = makeFork(i, false);
    if (err) {
      printf("ERROR: Can't fork!\n");
      return 1;
    }
  }

  signal(SIGHUP, hupHandler);

  err = handleProcesses();
  if (err) {
    printf("ERROR: Unexpected error with handle processes\n");
    return 2;
  }

  return 0;
}

int main(int argc, char *argv[]) {

  gargc = argc;
  gargv = argv;

  run(argc, argv);


  return 0;
}
