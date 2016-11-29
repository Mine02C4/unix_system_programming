/* 61414050 Naoya Niwa */

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void getargs(const char*, int, char*, int*, char**);
const char *getenv(const char *[], const char *);

#define MAX_LINE 256

int
main(const int margc, const char *margv[], const char *menvp[])
{
  char line[MAX_LINE];
  char argbuf[MAX_LINE * 2];
  while (1) {
    printf("$ ");
    fflush(stdout);
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      fprintf(stderr, "Input error when fgets.\n");
      return 1;
    }
    if (line[strlen(line) - 1] != '\n') {
      fprintf(stderr, "Input is too long. Max = %d.\n", MAX_LINE - 2);
      continue;
    }
    int argc;
    char *argv[MAX_LINE];
    getargs(line, MAX_LINE, argbuf, &argc, argv);
    int i;
    for (i = 0; i < argc; i++) {
      printf("'%s',", argv[i]);
    }
    putchar('\n');
    if (argc == 0) {
      continue;
    }
    if (strcmp(argv[0], "cd") == 0) {
      if (argc > 1) {
        if (chdir(argv[1]) != 0) {
          fprintf(stderr, "Error chdir: %s\n", strerror(errno));
        }
      } else {
        const char *home = getenv(menvp, "HOME");
        printf("%s\n", home);
        if (chdir(home) != 0) {
          fprintf(stderr, "Error chdir: %s\n", strerror(errno));
        }
      }
      continue;
    }
    if (strcmp(argv[0], "exit") == 0) {
      return 0;
    }
    int background = 0;
    if (argv[argc - 1][strlen(argv[argc - 1]) - 1] == '&') {
      background = 1;
      if (strlen(argv[argc - 1]) == 1) {
        argv[argc - 1] = NULL;
      } else {
        argv[argc - 1][strlen(argv[argc - 1]) - 1] = '\0';
        argv[argc] = NULL;
      }
    } else {
      argv[argc] = NULL;
    }
    int pid;
    if ((pid = fork()) != 0) {
      if (background == 0) {
        int status;
        wait(&status);
      }
    } else {
      execvp(argv[0], argv);
      fprintf(stderr, "Error execvp: %s\n", strerror(errno));
      return 0;
    }
  }
  return 0;
}

