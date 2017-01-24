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
  if (send(socket, &pkt, sizeof(pkt), 0) < 0) {
    fprintf(stderr, "Error in send: %s\n", strerror(errno));
    exit(1);
  }
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


