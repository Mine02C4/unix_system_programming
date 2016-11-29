/* 61414050 Naoya Niwa */

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
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
    int argc, i, error_flag = 0;
    char *argv[MAX_LINE];
    getargs(line, MAX_LINE, argbuf, &argc, argv);
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
        argv[--argc] = NULL;
      } else {
        argv[argc - 1][strlen(argv[argc - 1]) - 1] = '\0';
        argv[argc] = NULL;
      }
    } else {
      argv[argc] = NULL;
    }
    char *redirect_stdin = NULL;
    char *redirect_stdout = NULL;
    const static char *specialcs = "><|";
    for (i = 0; i < argc; i++) {
      int j;
      if (strcmp(argv[i], "<") == 0) {
        if (i + 1 >= argc) {
          fprintf(stderr, "Syntax error\n");
          error_flag = 1;
          break;
        }
        redirect_stdin = argv[i + 1];
        for (j = 0; specialcs[j] != '\0'; j++) {
          if (redirect_stdin[0] == specialcs[j]) {
            fprintf(stderr, "Syntax error\n");
            error_flag = 1;
            break;
          }
        }
        if (error_flag) {
          break;
        }
        argv[i] = NULL;
        argv[i + 1] = NULL;
        i++;
      } else if (strcmp(argv[i], ">") == 0) {
        if (i + 1 >= argc) {
          fprintf(stderr, "Syntax error\n");
          error_flag = 1;
          break;
        }
        redirect_stdout = argv[i + 1];
        for (j = 0; specialcs[j] != '\0'; j++) {
          if (redirect_stdout[0] == specialcs[j]) {
            fprintf(stderr, "Syntax error\n");
            error_flag = 1;
            break;
          }
        }
        if (error_flag) {
          break;
        }
        argv[i] = NULL;
        argv[i + 1] = NULL;
        i++;
      }
    }
    if (error_flag) {
      continue;
    }
    {
      int d = 0;
      for (i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
          d++;
          continue;
        }
        if (d > 0) {
          argv[i - d] = argv[i];
        }
      }
      argv[argc - d] = NULL;
    }
    for (i = 0; i < argc; i++) {
      printf("'%s',", argv[i]);
    }
    putchar('\n');
    int pid;
    if ((pid = fork()) != 0) {
      if (background == 0) {
        int status;
        wait(&status);
      }
    } else {
      if (redirect_stdin != NULL) {
        int infd = open(redirect_stdin, O_RDONLY);
        if (infd == -1) {
          fprintf(stderr, "Error stdin redirect: %s\n", strerror(errno));
          return errno;
        }
        close(0);
        if (dup(infd) == -1) {
          fprintf(stderr, "Error stdin redirect: %s\n", strerror(errno));
          return errno;
        }
      }
      if (redirect_stdout != NULL) {
        int outfd = open(redirect_stdout, O_WRONLY|O_CREAT|O_TRUNC, 0644);
        if (outfd == -1) {
          fprintf(stderr, "Error stdout redirect: %s\n", strerror(errno));
          return errno;
        }
        close(1);
        if (dup(outfd) == -1) {
          fprintf(stderr, "Error stdout redirect: %s\n", strerror(errno));
          return errno;
        }
      }
      execvp(argv[0], argv);
      fprintf(stderr, "Error execvp: %s\n", strerror(errno));
      return errno;
    }
  }
  return 0;
}

