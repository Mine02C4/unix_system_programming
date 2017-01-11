#ifndef MYDHCP_TYPES_H_
#define MYDHCP_TYPES_H_

#include <netinet/in.h>

struct dhcp_msg {
  uint8_t type;
  uint8_t code;
  uint16_t ttl;
  in_addr_t addr;
  in_addr_t mask;
};

#endif  // MYDHCP_TYPES_H_
