const static char *skipcs = " \n\t";
const static char *specialcs = "><|";

void
getargs(const char *input, int max, char *argbuf, int *argc, char *argv[])
{
  int i, j, bufi;
  *argc = 0;
  int npos = 0, prev_skip = 1, prev_special = 0;
  for (i = 0, bufi = 0; i < max; i++, bufi++) {
    argbuf[bufi] = input[i];
    if (input[i] == '\0') {
      if (prev_skip == 0) {
        argv[(*argc)++] = argbuf + npos;
      }
      break;
    }
    int skip = 0;
    for (j = 0; skipcs[j] != '\0'; j++) {
      if (input[i] == skipcs[j]) {
        skip = 1;
        break;
      }
    }
    int special = 0;
    if (!skip) {
      for (j = 0; specialcs[j] != '\0'; j++) {
        if (input[i] == specialcs[j]) {
          special = 1;
          break;
        }
      }
    }
    if ((special && !prev_skip && !prev_special) || (!special && prev_special && !skip)) {
      argbuf[bufi++] = '\0';
      argbuf[bufi] = input[i];
      argv[(*argc)++] = argbuf + npos;
      npos = bufi;
    } else if (!special && prev_special && !skip) {
    }
    if (skip && !prev_skip) {
      argv[(*argc)++] = argbuf + npos;
      argbuf[bufi] = '\0';
    } else if (!skip && prev_skip) {
      npos = bufi;
    }
    prev_skip = skip;
    prev_special = special;
  }
}

