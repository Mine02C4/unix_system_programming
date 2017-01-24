#include "client.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

#include "types.h"

#define MAX_LINE 1024

void getargs(const char *input, int max, int *argc, char *argv[]);

typedef void (* cmdfuncptr)(int, char**);

void quitcmd(int, char**);
//void pwdcmd(int, char**);

struct cmdinfo {
  char *name;
  cmdfuncptr func;
};

const struct cmdinfo cmdarray[] = {
  {"quit",    quitcmd},
//  {"pwd",     pwdcmd},
};

void
cmdline()
{
  char line[MAX_LINE];
  while (1) {
    printf("myFTP%% ");
    fflush(stdout);
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      fprintf(stderr, "Input error when fgets.\n");
      exit(1);
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
}


int s;

void
quitcmd(int argc, char** argv)
{
  close(s);
  exit(0);
}

void
setup_socket(const char *hostname)
{
  struct addrinfo hints, *res;
  int err;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  char strport[50];
  snprintf(strport, 50, "%d", MYFTP_SERVER_PORT);
  if ((err = getaddrinfo(hostname, strport, &hints, &res)) < 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    exit(1);
  }
  if ((s = socket(res->ai_family, res->ai_socktype, 0)) < 0) {
    fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
    exit(errno);
  }
  if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
    fprintf(stderr, "Error in connect: %s\n", strerror(errno));
    exit(errno);
  }
  printf("Server: %s: %d\n",
      inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr),
      ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port));
  freeaddrinfo(res);
}

int
main(const int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stderr, "Usase: ./myftpc <server-name>\n");
    exit(1);
  }
  setup_socket(argv[1]);
  cmdline();
  close(s);
  return 0;
}

