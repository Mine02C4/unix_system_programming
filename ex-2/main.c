#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define MAX_LINE 256

extern int buffer_size;

extern void getargs(char*, int, int*, char**);

extern void init_head();
extern void init_buffer();
extern void print_buffer(int);
extern void print_free();

void helpcmd(int, char**);
void initcmd(int, char**);
void bufcmd(int, char**);
void hashcmd(int, char**);
void freecmd(int, char**);
void getblkcmd(int, char**);
void brelsecmd(int, char**);
void setcmd(int, char**);
void resetcmd(int, char**);
void quitcmd(int, char**);

typedef void (* cmdfuncptr)(int, char**);

struct cmdinfo {
  char *name;
  cmdfuncptr func;
};

const struct cmdinfo cmdarray[] = {
  {"help",    helpcmd},
  {"init",    initcmd},
  {"buf",     bufcmd},
  {"hash",    hashcmd},
  {"free",    freecmd},
  {"getblk",  getblkcmd},
  {"brelse",  brelsecmd},
  {"set",     setcmd},
  {"reset",   resetcmd},
  {"quit",    quitcmd},
};

int
main()
{
  char line[MAX_LINE];
  init_head();
  while (1) {
    printf("$ ");
    fflush(stdout);
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      fprintf(stderr, "Input error when fgets.\n");
      return 1;
    }
    int argc;
    char *argv[MAX_LINE];
    getargs(line, MAX_LINE, &argc, argv);
    if (argc > 0 && argv[0][0] != '\0') {
      int i;
      int validcmd = 0;
      for (i = 0; i < sizeof(cmdarray) / sizeof(cmdarray[0]); i++) {
        if (strcmp(cmdarray[i].name, argv[0]) == 0) {
          validcmd = 1;
          cmdarray[i].func(argc, argv);
          break;
        }
      }
      if (!validcmd) {
        fprintf(stderr, "Unkown command '%s'.\n", argv[0]);
      }
    }
  }
  return 0;
}

void
helpcmd(int argc, char** argv)
{
  printf("Commands help\n\n");
  printf("help\n");
  printf("  Show this help.\n\n");
  printf("init\n");
  printf("  Initialize buffer cache.\n\n");
  printf("buf [n ...]\n");
  printf("  If no argument was given, show all buffer state.\n");
  printf("  If arguments were given, show buffers state which were shown by arguments 'n'.\n\n");
}

void
initcmd(int argc, char** argv)
{
  init_buffer();
}

void
bufcmd(int argc, char** argv)
{
  if (argc == 1) {
    int i;
    for (i = 0; i < buffer_size; i++) {
      print_buffer(i);
    }
  } else {
    int i;
    int *bufarr;
    if ((bufarr = (int *)malloc(sizeof(int) * (argc - 1))) == NULL) {
      fprintf(stderr, "Memory error!! malloc returns NULL!!\n");
      exit(1);
    }
    for (i = 1; i < argc; i++) {
      char *endp;
      int n = strtol(argv[i], &endp, 10);
      if (*endp == '\0') {
        if (n >= 0 && n < buffer_size) {
          bufarr[i - 1] = n;
        } else {
          fprintf(stderr, "Invalid argument: '%d' is out of range [0,%d].\n", n, buffer_size - 1);
          free(bufarr);
          return;
        }
      } else {
        fprintf(stderr, "Invalid argument: '%s' is not a number.\n", argv[i]);
        free(bufarr);
        return;
      }
    }
    for (i = 0; i < argc - 1; i++) {
      print_buffer(bufarr[i]);
    }
    free(bufarr);
  }
}

void
hashcmd(int argc, char** argv)
{
}

void
freecmd(int argc, char** argv)
{
  print_free();
}

void
getblkcmd(int argc, char** argv)
{
}

void
brelsecmd(int argc, char** argv)
{
}

void
setcmd(int argc, char** argv)
{
}

void
resetcmd(int argc, char** argv)
{
}

void
quitcmd(int argc, char** argv)
{
  exit(0);
}

