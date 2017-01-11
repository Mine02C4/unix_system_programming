#ifndef MYDHCP_POOL_H_
#define MYDHCP_POOL_H_

#include <netinet/in.h>

struct dhcp_addr {
  struct dhcp_addr *fp;
  struct dhcp_addr *bp;
  struct in_addr addr;
  struct in_addr netmask;
};

extern int dhcp_ttl;
extern struct dhcp_addr addr_pool;
extern void init_addr_pool(const char *filename);
extern void insert_addr_pool(struct dhcp_addr *da);
extern struct dhcp_addr *get_addr();

#endif  // MYDHCP_POOL_H_

