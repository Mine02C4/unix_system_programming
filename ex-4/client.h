#ifndef MYDHCP_CLIENT_H_
#define MYDHCP_CLIENT_H_

enum eStatus {
  Status_WaitOffer = 1,
  Status_ResentWaitOffer,
  Status_WaitAck,
  Status_ResentWaitAck,
  Status_InUse,
  Status_WaitExtAck,
  Status_ResentWaitExtAck
};

enum eEvent {
  Event_ReceiveOfferOK = 1,
  Event_ReceiveOfferNG,
  Event_ReceiveAckOK,
  Event_ReceiveAckNG,
  Event_SIGHUP,
  Event_HalfOfTTL,
  Event_ReceiveTimeout,
  Event_InvalidPacket
};

typedef void (* procfuncptr)(void);

struct proctable {
  enum eStatus status;
  enum eEvent event;
  procfuncptr func;
  enum eStatus next_status;
};

/* procfunc */
void send_discover();
void send_request_alloc();
void send_request_ext();
void send_release();

enum eEvent wait_event();

#endif  // MYDHCP_CLIENT_H_

