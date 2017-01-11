#include "server.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct proctable ptab[] = {
  { Status_WaitDiscover,  Event_ReceiveDiscover,        send_offer_ok,  Status_WaitRequest },
  { Status_WaitDiscover,  Event_ReceiveDiscoverNG,      send_offer_ng,  0 },
  { Status_WaitRequest,   Event_ReceiveRequestAllocNG,  send_ack_ng,    0 },
  { Status_WaitRequest,   Event_ReceiveRequestAllocOK,  send_ack_ok,    Status_InUse },
  { Status_WaitRequest,   Event_ReceiveTimeout,         send_offer_ok,  Status_ResentWaitRequest },
  { Status_ResentWaitRequest, Event_ReceiveRequestAllocNG, send_ack_ng, 0 },
  { Status_ResentWaitRequest, Event_ReceiveRequestAllocOK, send_ack_ok, Status_InUse },
  { Status_InUse,         Event_ReceiveRequestExtOK,    ttl_reset,      Status_InUse },
  { Status_InUse,         Event_ReceiveRequestExtNG,    send_ack_ng,    0 },
  { 0, 0, NULL, 0 }
};

struct client {
  struct client *fp;
  struct client *bp;
  enum eStatus status;
  int ttlcounter;
  struct in_addr id;
  struct in_addr addr;
  struct in_addr netmask;
  uint16_t ttl;
};

struct client client_list;

void
init_client_list()
{
  client_list.fp = &client_list;
  client_list.bp = &client_list;
}

struct client *
search_client(struct sockaddr_in *skt)
{
  struct client *cp;
  for (cp = client_list.fp; cp != &client_list; cp = cp->fp) {
    if (cp->id.s_addr == skt->sin_addr.s_addr) {
      return cp;
    }
  }
  return NULL;
}

struct client *
add_client(struct sockaddr_in *skt) {
  printf("add_client\n");
  struct client *cp;
  if ((cp = (struct client *)malloc(sizeof(struct client))) == NULL) {
    fprintf(stderr, "Error: Memory allocation error!!!!\n");
    exit(1);
  }
  cp->fp = &client_list;
  cp->bp = client_list.bp;
  cp->fp->bp = cp;
  cp->bp->fp = cp;
  return cp;
}

void
delete_client(struct client *cp)
{
  cp->fp->bp = cp->bp;
  cp->bp->fp = cp->fp;
  free(cp);
}

struct dhcp_addr {
  struct dhcp_addr *fp;
  struct dhcp_addr *bp;
  struct in_addr addr;
  struct in_addr netmask;
};

struct dhcp_addr addr_pool;
int dhcp_ttl;

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
    if (!(inet_aton(addr_str, &addr) && inet_aton(mask_str, &mask))) {
      fprintf(stderr, "Error: config-file invalid format.\n");
      exit(1);
    }
    struct dhcp_addr *da;
    if ((da = (struct dhcp_addr *)malloc(sizeof(struct dhcp_addr))) == NULL) {
      fprintf(stderr, "Error: Memory allocation error!!!!\n");
      exit(1);
    }
    insert_addr_pool(da);
  }
}

int s;
struct sockaddr_in myskt;
struct client *nowcl;
struct sockaddr_in skt;

int
main(const int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stderr, "Usase: ./mydhcpd <config-file>\n");
    exit(1);
  }
  init_addr_pool(argv[1]);

  init_client_list();

  in_port_t myport = MYDHCP_PORT_NUMBER;
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
    return errno;
  }
  memset(&myskt, 0, sizeof(myskt));
  myskt.sin_family = AF_INET;
  myskt.sin_port = htons(myport);
  myskt.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr *) &myskt, sizeof(myskt)) < 0) {
    fprintf(stderr, "Error in binding: %s\n", strerror(errno));
    return errno;
  }

  printf("Start listening.\n");

  struct proctable *pt;
  enum eEvent event;
  nowcl = NULL;
  for (;;) {
    event = wait_event();
    if (nowcl == NULL) {
      continue;
    }
    for (pt = ptab; pt->status; pt++) {
      if (pt->status == nowcl->status && pt->event == event) {
        (*pt->func)();
        nowcl->status = pt->next_status;
        break;
      }
    }
    if (pt->status == 0) {
      release_client();
    }
    nowcl = NULL;
  }
  return 0;
}

int
parse_message(char msg[], int length, struct dhcp_msg *dmsg)
{
  if (length != 12) {
    fprintf(stderr, "Invalid message size.\n");
    return -1;
  }
  memcpy(msg, dmsg, length);
  return 0;
}

enum eEvent
wait_event()
{
  int count;
  socklen_t sktlen = sizeof(skt);
  struct dhcp_msg msg;
  if ((count = recvfrom(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, &sktlen)) < 0) {
    fprintf(stderr, "Error: in recvfrom");
    exit(1);
  }
  printf("Recieve packet.\n");
  print_hex((char *)&msg, count);
  if (count != 12) {
    fprintf(stderr, "Invalid message size.\n");
    return Event_InvalidPacket;
  }
  nowcl = search_client(&skt);
  switch (msg.type) {
    case 1:
      printf("DISCOVER\n");
      if (msg.code == 0 && msg.ttl == 0 && msg.addr == 0 && msg.mask == 0) {
        if (nowcl == NULL) {
          nowcl = add_client(&skt);
        }
        printf("Recieve DISCOVER.\n");
        return Event_ReceiveDiscover;
      } else {
        return Event_InvalidPacket;
      }
    case 3:
      if (nowcl == NULL)
        return Event_InvalidPacket;
      switch (msg.code) {
        case 2:
          if (ntohs(msg.ttl) <= dhcp_ttl &&
              nowcl->addr.s_addr == msg.addr && nowcl->netmask.s_addr == msg.mask) {
            return Event_ReceiveRequestAllocOK;
          } else {
            return Event_ReceiveRequestAllocNG;
          }
        case 3:
          if (ntohs(msg.ttl) <= dhcp_ttl &&
              nowcl->addr.s_addr == msg.addr && nowcl->netmask.s_addr == msg.mask) {
            return Event_ReceiveRequestExtOK;
          } else {
            return Event_ReceiveRequestExtNG;
          }
        default:
          return Event_InvalidPacket;
      }
      break;
    case 5:
      if (nowcl == NULL)
        return Event_InvalidPacket;
      if (msg.code == 0 && msg.ttl == 0 && msg.mask == 0 && nowcl->addr.s_addr == msg.addr) {
        return Event_ReceiveRelease;
      } else {
        return Event_InvalidPacket;
      }
      break;
    default:
      return Event_InvalidPacket;
  }
  return Event_InvalidPacket;
}


void
send_offer_ok()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 2;
  msg.code = 0;
  msg.ttl = nowcl->ttl;
  msg.addr = nowcl->addr.s_addr;
  msg.mask = nowcl->addr.s_addr;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
}

void
send_offer_ng()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 2;
  msg.code = 1;
  msg.ttl = 0;
  msg.addr = 0;
  msg.mask = 0;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
  release_client();
}

void
send_ack_ok()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 4;
  msg.code = 0;
  msg.ttl = nowcl->ttl;
  msg.addr = nowcl->addr.s_addr;
  msg.mask = nowcl->netmask.s_addr;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
}

void
send_ack_ng()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 4;
  msg.code = 4;
  msg.ttl = 0;
  msg.addr = 0;
  msg.mask = 0;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
  release_client();
}

void
ttl_reset()
{
  // TODO: reset...?
  send_ack_ok();
}

void
release_client()
{
  if (nowcl != NULL) {
    // TODO: implement
  }
}

