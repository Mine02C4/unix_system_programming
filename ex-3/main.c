/* 61414050 Naoya Niwa */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void getargs(const char*, int, char*, int*, char**);
int execmanual(const char *filename, char *const argv[], char *envp[]);
void new_prompt();

#define MAX_LINE 256
#define MAX_PIPE 100

struct child_proc {
  char *redirect_stdin;
  char *redirect_stdout;
  char **argv;
};

int reset_flag = 0;

void
new_prompt()
{
  printf("$ ");
  fflush(stdout);
}

void
sigint_handler(int signum)
{
  reset_flag = 2;
}

void
sigchld_handler(int signum)
{
  reset_flag = 1;
  if (signum == SIGCHLD) {
    int status;
    wait(&status);
  }
}

int
main(const int margc, const char *margv[], char *menvp[])
{
  char line[MAX_LINE];
  char argbuf[MAX_LINE * 2];
  struct child_proc childs[MAX_PIPE];
  struct sigaction act;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags |= SA_RESTART;
  if (sigaction(SIGINT, &act, NULL) == -1) {
    fprintf(stderr, "Error sigaction: %s\n", strerror(errno));
    return errno;
  }
  act.sa_handler = sigchld_handler;
  if (sigaction(SIGCHLD, &act, NULL) == -1) {
    fprintf(stderr, "Error sigaction: %s\n", strerror(errno));
    return errno;
  }
  while (1) {
    if (reset_flag == 2) {
      reset_flag = 0;
      putchar('\n');
    }
    if (reset_flag) {
      reset_flag = 0;
    }
    new_prompt();
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      if (reset_flag) {
        continue;
      }
      if (feof(stdin)) {
        printf("exit\n");
        return 0;
      }
      fprintf(stderr, "Input error when fgets. %d\n", ferror(stdin));
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
        const char *home = getenv("HOME");
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
    const static char *specialcs = "><|";
    int pipe_n = 0;
    childs[0].redirect_stdin = NULL;
    childs[0].redirect_stdout = NULL;
    childs[0].argv = argv;
    for (i = 0; i < argc; i++) {
      int j;
      int special_flag = 0;
      for (j = 0; specialcs[j] != '\0'; j++) {
        if (argv[i][0] == specialcs[j]) {
          special_flag = 1;
          break;
        }
      }
      if (special_flag) {
        if (strcmp(argv[i], "<") == 0) {
          if (i + 1 >= argc) {
            fprintf(stderr, "Syntax error\n");
            error_flag = 1;
            break;
          }
          char *r_stdin = argv[i + 1];
          for (j = 0; specialcs[j] != '\0'; j++) {
            if (r_stdin[0] == specialcs[j]) {
              fprintf(stderr, "Syntax error\n");
              error_flag = 1;
              break;
            }
          }
          if (error_flag) {
            break;
          }
          childs[pipe_n].redirect_stdin = r_stdin;
          argv[i] = NULL;
          argv[i + 1] = NULL;
          i++;
        } else if (strcmp(argv[i], ">") == 0) {
          if (i + 1 >= argc) {
            fprintf(stderr, "Syntax error\n");
            error_flag = 1;
            break;
          }
          char *r_stdout = argv[i + 1];
          for (j = 0; specialcs[j] != '\0'; j++) {
            if (r_stdout[0] == specialcs[j]) {
              fprintf(stderr, "Syntax error\n");
              error_flag = 1;
              break;
            }
          }
          if (error_flag) {
            break;
          }
          childs[pipe_n].redirect_stdout = r_stdout;
          argv[i] = NULL;
          argv[i + 1] = NULL;
          i++;
        } else if (strcmp(argv[i], "|") == 0) {
          for (j = 0; specialcs[j] != '\0'; j++) {
            if (argv[i + 1][0] == specialcs[j]) {
              fprintf(stderr, "Syntax error\n");
              error_flag = 1;
              break;
            }
          }
          pipe_n++;
          if (error_flag) {
            break;
          }
          childs[pipe_n].redirect_stdin = NULL;
          childs[pipe_n].redirect_stdout = NULL;
          childs[pipe_n].argv = &argv[i + 1];
        } else {
          fprintf(stderr, "Syntax error\n");
          error_flag = 1;
          break;
        }
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
        if (strcmp(argv[i], "|") == 0) {
          argv[i - d] = NULL;
          continue;
        }
        if (d > 0) {
          argv[i - d] = argv[i];
        }
      }
      argv[argc - d] = NULL;
    }
    int pipes[MAX_PIPE][2];
    for (i = 0; i < pipe_n; i++) {
      if (pipe(pipes[i]) == -1) {
        fprintf(stderr, "Error open pipe: %s\n", strerror(errno));
        return errno;
      }
    }
    for (i = 0; i <= pipe_n; i++) {
      int pid;
      if ((pid = fork()) != 0) {
        continue;
      } else {
        if (i > 0) {
          close(0);
          if (dup(pipes[i - 1][0]) == -1) {
            fprintf(stderr, "Error dup pipe to stdin: %s\n", strerror(errno));
            return errno;
          }
        }
        if (i < pipe_n) {
          close(1);
          if (dup(pipes[i][1]) == -1) {
            fprintf(stderr, "Error dup pipe to stdout: %s\n", strerror(errno));
            return errno;
          }
        }
        int j;
        for (j = 0; j < pipe_n; j++) {
          close(pipes[j][0]);
          close(pipes[j][1]);
        }
        if (childs[i].redirect_stdin != NULL) {
          int infd = open(childs[i].redirect_stdin, O_RDONLY);
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
        if (childs[i].redirect_stdout != NULL) {
          int outfd = open(childs[i].redirect_stdout, O_WRONLY|O_CREAT|O_TRUNC, 0644);
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
        execmanual(childs[i].argv[0], childs[i].argv, menvp);
        fprintf(stderr, "Error execvp: %s\n", strerror(errno));
        return errno;
      }
    }
    for (i = 0; i < pipe_n; i++) {
      close(pipes[i][0]);
      close(pipes[i][1]);
    }
    if (background == 0) {
      int status;
      for (i = 0; i <= pipe_n; i++) {
        wait(&status);
      }
    }
  }
  return 0;
}

