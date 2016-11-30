#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 1024

int
execmanual(const char *filename, char *const argv[], char *envp[])
{
  if (*filename == '\0') {
    return -1;
  }
  if (strchr(filename, '/') != NULL) {
    execve(filename, argv, envp);
    return -1;
  } else {
    char *path = getenv("PATH");
    char pbuf[PATH_MAX];
    int i;
    char *p = path;
    for (i = 0; ; i++) {
      if (path[i] == ':' || path[i] == '\0') {
        int l = path + i - p;
        strncpy(pbuf, p, l);
        pbuf[l] = '/';
        pbuf[l + 1] = '\0';
        strncat(pbuf, filename, PATH_MAX - l - 2);
        execve(pbuf, argv, envp);
        p = path + i + 1;
      }
      if (path[i] == '\0') {
        break;
      }
    }
  }
  errno = ENOENT;
  return -1;
}

