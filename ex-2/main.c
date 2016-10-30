#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define MAX_LINE 256

extern int buffer_size;

extern void getargs(char*, int, int*, char**);
extern struct buf_header *getblk(int);
extern void brelse(struct buf_header *);

extern void init_head();
extern void init_buffer();
extern void print_buffer(int);
extern void print_hash(int);
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
  if (argc == 1) {
    int i;
    for (i = 0; i < NHASH; i++) {
      print_hash(i);
    }
  } else {
    int i;
    int *hasharr;
    if ((hasharr = (int *)malloc(sizeof(int) * (argc - 1))) == NULL) {
      fprintf(stderr, "Memory error!! malloc returns NULL!!\n");
      exit(1);
    }
    for (i = 1; i < argc; i++) {
      char *endp;
      int n = strtol(argv[i], &endp, 10);
      if (*endp == '\0') {
        if (n >= 0 && n < NHASH) {
          hasharr[i - 1] = n;
        } else {
          fprintf(stderr, "Invalid argument: '%d' is out of range [0,%d].\n", n, NHASH - 1);
          free(hasharr);
          return;
        }
      } else {
        fprintf(stderr, "Invalid argument: '%s' is not a number.\n", argv[i]);
        free(hasharr);
        return;
      }
    }
    for (i = 0; i < argc - 1; i++) {
      print_buffer(hasharr[i]);
    }
    free(hasharr);
  }
}

void
freecmd(int argc, char** argv)
{
  print_free();
}

void
getblkcmd(int argc, char** argv)
{
  if (argc == 2) {
    char *endp;
    int n = strtol(argv[1], &endp, 10);
    if (*endp == '\0') {
      struct buf_header *p;
      if ((p = getblk(n)) == NULL) {
        fprintf(stderr, "getblk(%d) returns NULL.\n", n);
      }
    } else {
      fprintf(stderr, "Invalid argument: '%s' is not a number.\n", argv[1]);
    }
  } else {
    fprintf(stderr, "Invalid argument: Command '%s' requires 1 parameter. Please see help.\n", argv[0]);
  }
}

void
brelsecmd(int argc, char** argv)
{
  if (argc == 2) {
    char *endp;
    int n = strtol(argv[1], &endp, 10);
    if (*endp == '\0') {
      struct buf_header *p;
      if ((p = hash_search(n)) == NULL) {
        fprintf(stderr, "Not found: Block '%d' is not found.\n", n);
      } else {
        brelse(p);
      }
    } else {
      fprintf(stderr, "Invalid argument: '%s' is not a number.\n", argv[1]);
    }
  } else {
    fprintf(stderr, "Invalid argument: Command '%s' requires 1 parameter. Please see help.\n", argv[0]);
  }
}

void
setcmd(int argc, char** argv)
{
  if (argc >= 3) {
    char *endp;
    int n = strtol(argv[1], &endp, 10);
    if (*endp == '\0') {
      struct buf_header *p;
      if ((p = hash_search(n)) == NULL) {
        fprintf(stderr, "Not found: Block '%d' is not found.\n", n);
      } else {
        int i, j;
        unsigned int s = 0x0;
        for (i = 2; i < argc; i++) {
          int len = strlen(argv[i]);
          for (j = 0; j < len; j++) {
            switch (argv[i][j]) {
              case 'L':
                s |= STAT_LOCKED;
                break;
              case 'V':
                s |= STAT_VALID;
                break;
              case 'D':
                s |= STAT_DWR;
                break;
              case 'K':
                s |= STAT_KRDWR;
                break;
              case 'W':
                s |= STAT_WAITED;
                break;
              case 'O':
                s |= STAT_OLD;
                break;
              default:
                fprintf(stderr, "Invalid argument: '%c' is unknown stat. Please see help.\n", argv[i][j]);
                return;
            }
          }
          p->stat |= s;
        }
      }
    } else {
      fprintf(stderr, "Invalid argument: '%s' is not a number.\n", argv[1]);
    }
  } else {
    fprintf(stderr, "Invalid argument: Command '%s' requires at least 2 parameters. Please see help.\n",
        argv[0]);
  }
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

