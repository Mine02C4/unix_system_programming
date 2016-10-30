#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

extern void getargs(char*, int, int*, char**);
extern void init_buffer();

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
  {"free",    initcmd},
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
    int i;
    printf("argc = %d\n", argc);
    for (i = 0; i < argc; i++) {
      printf("%s\n", argv[i]);
    }
    if (argc > 0 && argv[0][0] != '\0') {
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
}

void
hashcmd(int argc, char** argv)
{
}

void
freecmd(int argc, char** argv)
{
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

