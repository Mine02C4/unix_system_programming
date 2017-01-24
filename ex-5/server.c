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
#include <arpa/inet.h>

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
  printf("Connected from: %s: %d\n", inet_ntoa(skt->sin_addr), ntohs(skt->sin_port));
}

void
start_server(int sd)
{
  printf("Start server\n");
  struct myftph_data pkt;
  size_t header_size = sizeof(struct myftph);
  size_t recv_size;
  for (;;) {
    if ((recv_size = recv(sd, &pkt, header_size, 0)) < 0) {
      fprintf(stderr, "Error in recv: %s\n", strerror(errno));
      exit(errno);
    }
    if (recv_size == 0) {
      fprintf(stderr, "Connection refused.\n");
      exit(1);
    }
    if (recv_size != header_size) {
      fprintf(stderr, "Error: Invalid header size = %ld\n", recv_size);
      // TODO; CUT
      continue;
    }
    printf("recv pkt: data length = %d\n", pkt.length);
    print_hex((unsigned char *)&pkt, recv_size);
    if (pkt.length > 0) {
      int c = 0;
      for (;;) {
        if ((recv_size = recv(sd, ((void*) &pkt) + header_size + c, pkt.length - c, 0)) < 0) {
          fprintf(stderr, "Error in recv data: %s\n", strerror(errno));
          exit(errno);
        }
        if (recv_size == 0) {
          fprintf(stderr, "Connection refused.\n");
          exit(1);
        }
        c += recv_size;
        if (c == pkt.length) {
          break;
        } else if (c < pkt.length) {
          continue;
        } else {
          fprintf(stderr, "Error: oversize\n"); // unreachable
          // TODO: CUT
        }
      }
    }
    switch (pkt.type) {
      case TYPE_PWD:
        {
          printf("PWD\n");
          MYFTPDATA(pkt, TYPE_OK, CODE_OK);
          if (getcwd(pkt.data, MAX_DATASIZE) == NULL) {
            fprintf(stderr, "Error in getcwd: %s\n", strerror(errno));
            // TODO : CUT
            exit(errno);
          }
          pkt.length = strlen(pkt.data);
          printf("strlen = %ld\n dir = %s\n", strlen(pkt.data), pkt.data);
          print_hex((unsigned char *)&pkt, sizeof(struct myftph) + pkt.length);
          send_mydata(sd, &pkt);
          break;
        }
    }
  }
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
      start_server(s2);
      close(s2);
      break;
    } else { // Parent
      close(s2);
    }
  }
  return 0;
}


