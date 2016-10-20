#include <stdio.h>

#define MAX_LINE 256

extern void getargs(char*, int, int*, char**);

int main()
{
  char line[MAX_LINE];
  while (1) {
    printf("$ ");
    fflush(stdout);
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      // TODO: error
      fprintf(stderr, "Error\n");
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
  }
  return 0;
}

