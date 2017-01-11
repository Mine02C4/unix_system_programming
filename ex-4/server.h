#ifndef MYDHCP_SERVER_H_
#define MYDHCP_SERVER_H_

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

struct proctable {
  enum eStatus status;
  enum eEvent event;
  procfuncptr func;
};

/* procfunc */
void send_offer();
void release_client();

enum eEvent wait_event();

#endif  // MYDHCP_SERVER_H_

