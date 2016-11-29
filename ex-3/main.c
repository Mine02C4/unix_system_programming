/* 61414050 丹羽直也 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void getargs(char*, int, int*, char**);
const char *getenv(const char *[], const char *);

#define MAX_LINE 256

int
main(const int margc, const char *margv[], const char *menvp[])
{
  char line[MAX_LINE];
  while (1) {
    printf("%% ");
    fflush(stdout);
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      fprintf(stderr, "Input error when fgets.\n");
      return 1;
    }
    int argc;
    char *argv[MAX_LINE];
    getargs(line, MAX_LINE, &argc, argv);
    if (argc == 0) {
      continue;
    }
    if (strcmp(argv[0], "cd") == 0) {
      if (argc > 1) {
        chdir(argv[1]);
      } else {
        const char *home = getenv(menvp, "HOME");
        printf("%s\n", home);
        chdir(home);
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
    }
  }
  return 0;
}

