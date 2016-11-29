#include <stddef.h>

const char *
getenv(const char *envp[], const char *key)
{
  int i, j;
  for (i = 0; envp[i] != NULL; i++) {
    const char *val = NULL;
    int n = 0;
    for (j = 0; envp[j] != '\0'; j++) {
      if (envp[i][j] == '=') {
        val = &envp[i][j + 1];
        n = j;
        break;
      }
    }
    if (val == NULL) {
      break;
    }
    if (strncmp(envp[i], key, n) == 0) {
      return val;
    }
  }
  return NULL;
}

