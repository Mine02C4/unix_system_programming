#ifndef MYDHCP_TYPES_H_
#define MYDHCP_TYPES_H_

#include <netinet/in.h>

#define MYDHCP_PORT_NUMBER 51230

struct dhcp_msg {
  uint8_t type;
  uint8_t code;
  uint16_t ttl;
  in_addr_t addr;
  in_addr_t mask;
};

#include <stdio.h>

void
print_hex(const char *data, int length)
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

#endif  // MYDHCP_TYPES_H_
