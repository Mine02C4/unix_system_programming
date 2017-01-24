#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

void
myftph_init(struct myftph *pkt, uint8_t type, uint8_t code)
{
  pkt->type = type;
  pkt->code = code;
  pkt->length = 0;
}

void
myftph_data_init(struct myftph_data *pkt, uint8_t type, uint8_t code)
{
  pkt->type = type;
  pkt->code = code;
  pkt->length = 0;
}

void
send_mypkt(int socket, struct myftph *pkt)
{
  if (send(socket, pkt, sizeof(*pkt), 0) < 0) {
    fprintf(stderr, "Error in send: %s\n", strerror(errno));
    exit(1);
  }
}

void
send_mydata(int socket, struct myftph_data *pkt)
{
  size_t pkt_size = sizeof(struct myftph) + pkt->length;
  if (send(socket, pkt, pkt_size, 0) < 0) {
    fprintf(stderr, "Error in send: %s\n", strerror(errno));
    exit(1);
  }
}

int
recv_myftp(int socket, struct myftph_data *pkt)
{
  size_t recv_size;
  const size_t header_size = sizeof(struct myftph);
  if ((recv_size = recv(socket, pkt, header_size, 0)) < 0) {
    fprintf(stderr, "Error in recv: %s\n", strerror(errno));
    exit(errno);
  }
  if (recv_size != header_size) {
    fprintf(stderr, "Error: Invalid header size\n");
    return -1;
  }
  printf("recv pkt: data length = %d\n", pkt->length);
  print_hex((unsigned char *)pkt, recv_size);
  if (pkt->length > 0) {
    int c = 0;
    for (;;) {
      if ((recv_size = recv(socket, ((char*) pkt) + header_size + c, pkt->length - c, 0)) < 0) {
        fprintf(stderr, "Error in recv data: %s\n", strerror(errno));
        exit(errno);
      }
      c += recv_size;
      printf("recv data c = %d\n", c);
      print_hex((unsigned char *)pkt, header_size + c);
      if (c == pkt->length) {
        break;
      } else if (c < pkt->length) {
        continue;
      } else {
        fprintf(stderr, "Error: oversize\n"); // unreachable
        return -1;
      }
    }
    printf("recv with data length = %d\n", c);
    print_hex((unsigned char *)pkt, header_size + c);
  }
  return 0;
}

void
print_hex(const unsigned char *data, int length)
{
  int i;
  const int width = 4;
  for (i = 0; i < length; i++) {
    printf("%02x ", data[i]);
    if (i % width == width - 1)
      putchar('\n');
  }
  putchar('\n');
}


