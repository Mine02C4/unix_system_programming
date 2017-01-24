#ifndef MYFTP_TYPES_H_
#define MYFTP_TYPES_H_

#define MYFTP_SERVER_PORT 50021
#define MAX_DATASIZE 1024

#include <stdint.h>

struct myftph {
  uint8_t type;
  uint8_t code;
  uint16_t length;
};

struct myftph_data {
  uint8_t type;
  uint8_t code;
  uint16_t length;
  char data[MAX_DATASIZE];
};

#define MYFTPPKT(x, t, c) struct myftph x; myftph_init(&x, t, c);
#define MYFTPDATA(x, t, c) struct myftph_data x; myftph_data_init(&x, t, c);

extern void myftph_init(struct myftph *pkt, uint8_t type, uint8_t code);
extern void myftph_data_init(struct myftph_data *pkt, uint8_t type, uint8_t code);

#endif  // MYFTP_TYPES_H_

