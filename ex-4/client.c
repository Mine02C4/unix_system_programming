#include "client.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct proctable ptab[] = {
  { 0, 0, NULL }
};

int s;

int
main(const int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stderr, "Usase: ./mydhcpdc <server-IP-address>\n");
    exit(1);
  }
  struct in_addr server_addr;
  if (!inet_aton(argv[1], &server_addr)) {
    fprintf(stderr, "Error: invalid server IP address.\n");
    exit(1);
  }

  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
    return errno;
  }

  int count;
  struct sockaddr_in skt;
  //char sbuf[512];
  struct dhcp_msg disc;
  in_port_t server_port = MYDHCP_PORT_NUMBER;
  disc.type = 1;
  disc.code = 0;
  disc.ttl = 0;
  disc.addr = 0;
  disc.mask = 0;
  skt.sin_family = AF_INET;
  skt.sin_port = htons(server_port);
  skt.sin_addr.s_addr = htonl(server_addr.s_addr);
  if ((count = sendto(s, &disc, sizeof(struct dhcp_msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
  return 0;
}

void
send_discover()
{
  ;
}

