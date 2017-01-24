#include <stdio.h>

char *skipcs = " \n\t";

void
getargs(char *input, int max, int *argc, char *argv[])
{
  int i;
  *argc = 0;
  int npos = 0, prev_skip = 1;
  for (i = 0; i < max; i++) {
    if (input[i] == '\0') {
      if (prev_skip == 0) {
        argv[(*argc)++] = input + npos;
      }
      break;
    }
    int skip = 0, j;
    for (j = 0; skipcs[j] != '\0'; j++) {
      if (input[i] == skipcs[j]) {
        skip = 1;
        break;
      }
    }
    if (skip != 0 && prev_skip == 0) {
      argv[(*argc)++] = input + npos;
      input[i] = '\0';
    } else if (skip == 0 && prev_skip != 0) {
      npos = i;
    }
    prev_skip = skip;
  }
}

