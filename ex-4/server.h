#ifndef MYDHCP_SERVER_H_
#define MYDHCP_SERVER_H_

enum eStatus {
  Status_WaitDiscover = 1,
  Status_WaitRequest,
  Status_ResentWaitRequest,
  Status_InUse,
};

enum eEvent {
  Event_ReceiveDiscover = 1,
  Event_ReceiveDiscoverNG,
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

struct proctable {
  enum eStatus status;
  enum eEvent event;
  procfuncptr func;
  enum eStatus next_status;
};

/* procfunc */
void send_offer_ok();
void send_offer_ng();
void send_ack_ng();
void send_ack_ok();
void ttl_reset();
void release_client();

enum eEvent wait_event();

#endif  // MYDHCP_SERVER_H_

