#include "client.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct proctable ptab[] = {
  { Status_WaitOffer, Event_ReceiveOfferOK, send_request_alloc, Status_WaitAck },
  { Status_WaitOffer, Event_ReceiveTimeout, send_discover,      Status_ResentWaitAck },
  { Status_WaitOffer, Event_ReceiveTimeout, send_discover,      Status_ResentWaitAck },
  { Status_ResentWaitOffer, Event_ReceiveOfferOK, send_request_alloc, Status_WaitAck },
  { Status_WaitAck,   Event_ReceiveAckOK,   NULL,               Status_InUse },
  { Status_WaitAck,   Event_ReceiveTimeout, send_request_alloc, Status_ResentWaitAck },
  { Status_ResentWaitAck, Event_ReceiveAckOK, NULL,             Status_InUse },
  { Status_InUse,     Event_HalfOfTTL,      send_request_ext,   Status_WaitExtAck },
  { Status_InUse,     Event_SIGHUP,         send_release,       0 },
  { Status_WaitExtAck, Event_ReceiveAckOK,  NULL,               Status_InUse },
  { Status_WaitExtAck, Event_ReceiveTimeout, send_request_ext,  Status_ResentWaitExtAck },
  { Status_ResentWaitExtAck, Event_ReceiveAckOK, NULL,          Status_InUse },
  { 0, 0, NULL, 0 }
};

int s;
struct sockaddr_in skt;
enum eStatus status;
int dhcp_ttl;
struct in_addr dhcp_addr;
struct in_addr dhcp_mask;

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

  in_port_t server_port = MYDHCP_PORT_NUMBER;
  skt.sin_family = AF_INET;
  skt.sin_port = htons(server_port);
  skt.sin_addr = server_addr;
  printf("Server: %s: %d\n", inet_ntoa(skt.sin_addr), server_port);

  send_discover();
  status = Status_WaitOffer;
  struct proctable *pt;
  enum eEvent event;
  for (;;) {
    event = wait_event();
    for (pt = ptab; pt->status; pt++) {
      if (pt->status == status && pt->event == event) {
        (*pt->func)();
        status = pt->next_status;
        break;
      }
    }
    if (pt->status == 0) {
      printf("Finish\n");
      break;
    }
  }
  return 0;
}

enum eEvent
wait_event()
{
  int count;
  struct sockaddr_in skt;
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
  switch (msg.type) {
    case 2:
      printf("Recieve OFFER\n");
      if (msg.code == 0) {
        return Event_ReceiveOfferOK;
      } else if (msg.code == 1){
        return Event_ReceiveOfferNG;
      } else {
        return Event_InvalidPacket;
      }
    default:
      return Event_InvalidPacket;
  }
}

void
send_discover()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 1;
  msg.code = 0;
  msg.ttl = 0;
  msg.addr = 0;
  msg.mask = 0;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
}

void
send_request_alloc()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 3;
  msg.code = 2;
  msg.ttl = dhcp_ttl;
  msg.addr = dhcp_addr.s_addr;
  msg.mask = dhcp_mask.s_addr;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
}

void
send_request_ext()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 3;
  msg.code = 3;
  msg.ttl = dhcp_ttl;
  msg.addr = dhcp_addr.s_addr;
  msg.mask = dhcp_mask.s_addr;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
}

void
send_release()
{
  int count;
  struct dhcp_msg msg;
  msg.type = 5;
  msg.code = 0;
  msg.ttl = 0;
  msg.addr = dhcp_addr.s_addr;
  msg.mask = 0;
  if ((count = sendto(s, &msg, sizeof(msg), 0, (struct sockaddr *)&skt, sizeof(skt))) < 0) {
    fprintf(stderr, "Error: in sendto");
    exit(1);
  }
}

