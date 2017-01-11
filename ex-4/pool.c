#include "pool.h"

#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>

int dhcp_ttl;
struct dhcp_addr addr_pool;

void
insert_addr_pool(struct dhcp_addr *da)
{
  da->fp = &addr_pool;
  da->bp = addr_pool.bp;
  da->fp->bp = da;
  da->bp->fp = da;
}

void
init_addr_pool(const char *filename)
{
  FILE* fp;
  if((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Error: config-file open error\n");
    exit(1);
  }
  if (fscanf(fp, "%d", &dhcp_ttl) != 1) {
    fprintf(stderr, "Error: config-file error in line 1\n");
    exit(1);
  }
  addr_pool.fp = &addr_pool;
  addr_pool.bp = &addr_pool;
  for (;;) {
    struct in_addr addr;
    struct in_addr mask;
    char addr_str[512], mask_str[512];
    if (fscanf(fp, "%s %s", addr_str, mask_str) == EOF) {
      break;
    }
    if (inet_aton(addr_str, &addr) == 0 || inet_aton(mask_str, &mask) == 0) {
      fprintf(stderr, "Error: config-file invalid format.\n");
      exit(1);
    }
    struct dhcp_addr *da;
    if ((da = (struct dhcp_addr *)malloc(sizeof(struct dhcp_addr))) == NULL) {
      fprintf(stderr, "Error: Memory allocation error!!!!\n");
      exit(1);
    }
    da->addr = addr;
    da->netmask = mask;
    insert_addr_pool(da);
  }
}

struct dhcp_addr *
get_addr()
{
  if (addr_pool.fp == &addr_pool) {
    return NULL;
  }
  struct dhcp_addr *da = addr_pool.fp;
  printf("da addr %s\n", inet_ntoa(da->addr));
  da->fp->bp = da->bp;
  da->bp->fp = da->fp;
  return da;
}

