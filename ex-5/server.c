#include "server.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int s;

void
setup_socket()
{
  in_port_t myport = MYFTP_SERVER_PORT;
  struct sockaddr_in myskt;
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
    exit(errno);
  }
  memset(&myskt, 0, sizeof(myskt));
  myskt.sin_family = AF_INET;
  myskt.sin_port = htons(myport);
  myskt.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr *) &myskt, sizeof(myskt)) < 0) {
    fprintf(stderr, "Error in binding: %s\n", strerror(errno));
    exit(errno);
  }
  if (listen(s, 5) < 0) {
    fprintf(stderr, "Error in listen: %s\n", strerror(errno));
    exit(errno);
  }
}

void
wait_connect(struct sockaddr_in *skt, int *socket)
{
  int s2;
  socklen_t sktlen = sizeof(*skt);
  if ((s2 = accept(s, (struct sockaddr *)skt, &sktlen)) < 0) {
    fprintf(stderr, "Error in accept: %s\n", strerror(errno));
    exit(errno);
  }
  *socket = s2;
}

int
main(const int argc, const char *argv[])
{
  setup_socket();
  for (;;) {
    struct sockaddr_in skt;
    int s2;
    wait_connect(&skt, &s2);
    int pid = fork();
    if (pid < 0) {
      fprintf(stderr, "Error in fork: %s\n", strerror(errno));
      exit(errno);
    } else if (pid == 0) { // Child
      close(s);
      const char *buf = "pyonpyon\n";
      send(s2, buf, strlen(buf), 0);
      close(s2);
      break;
    } else { // Parent
      close(s2);
    }
  }
  return 0;
}


