#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYDHCP_PORT_NUMBER 51230

enum eStatus {
  Status_WaitDiscover,
  Status_WaitRequest,
  Status_ResentWaitRequest,
  Status_InUse,
};

enum eEvent {
  Event_ReceiveDiscover,
  Event_ReceiveRequestAllocOK,
  Event_ReceiveRequestAllocNG,
  Event_ReceiveRequestExtOK,
  Event_ReceiveRequestExtNG,
  Event_ReceiveRelease,
  Event_ReceiveTimeout,
  Event_TTLTimeout,
  Event_InvalidPacket,
};

typedef void (* procfuncptr)(void);

void send_offer();

enum eEvent wait_event();

struct proctable {
  enum eStatus status;
  enum eEvent event;
  procfuncptr func;
};

struct proctable ptab[] = {
  {Status_WaitDiscover, Event_ReceiveDiscover, send_offer}
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

int
main(const int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stderr, "Usase: ./mydhcpd <config-file>\n");
    exit(1);
  }
  init_addr_pool(argv[1]);

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
  enum eStatus status = Status_WaitDiscover;
  for (;;) {
    event = wait_event();
    for (pt = ptab; pt->status; pt++) {
      if (pt->status == status && pt->event == event) {
        (*pt->func)();
        break;
      }
    }
    if (pt->status == 0) {
      // TODO: close session
      break;
    }
  }
  return 0;
}

struct dhcp_msg {
  uint8_t type;
  uint8_t code;
  uint16_t ttl;
  in_addr_t addr;
  in_addr_t mask;
};

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
  struct sockaddr_in skt;
  char rbuf[512];
  socklen_t sktlen = sizeof(skt);
  if ((count = recvfrom(s, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&skt, &sktlen)) < 0) {
    fprintf(stderr, "Error: in recvfrom");
    exit(1);
  }
  struct dhcp_msg msg;
  if (parse_message(rbuf, count, &msg) < 0) {
    return Event_InvalidPacket;
  }
  switch (msg.type) {
    case 1:
      if (msg.code == 0 && msg.ttl == 0 && msg.addr == 0 && msg.mask == 0) {
        return Event_ReceiveDiscover;
      } else {
        return Event_InvalidPacket;
      }
    case 3:
      switch (msg.code) {
        case 2:
          if (ntohs(msg.ttl) <= dhcp_ttl) { // TODO: IP check
            return Event_ReceiveRequestAllocOK;
          } else {
            return Event_ReceiveRequestAllocNG;
          }
        case 3:
          if (ntohs(msg.ttl) <= dhcp_ttl) { // TODO: IP check
            return Event_ReceiveRequestExtOK;
          } else {
            return Event_ReceiveRequestExtNG;
          }
        default:
          return Event_InvalidPacket;
      }
      break;
    case 5:
        if (msg.code == 0 && msg.ttl == 0 && msg.mask == 0) {
          // TODO: IP check
          return Event_ReceiveRelease;
        } else {
          return Event_InvalidPacket;
        }
      break;
    default:
      return Event_InvalidPacket;
  }
  return -1;
}


void
send_offer()
{

}

