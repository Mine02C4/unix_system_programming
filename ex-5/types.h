#ifndef MYFTP_TYPES_H_
#define MYFTP_TYPES_H_

#define MYFTP_SERVER_PORT 50021
#define MAX_DATASIZE 1024

#include <stdint.h>

#define TYPE_QUIT     0x01
#define TYPE_PWD      0x02
#define TYPE_CWD      0x03
#define TYPE_LIST     0x04
#define TYPE_RETR     0x05
#define TYPE_STOR     0x06
#define TYPE_OK       0x10
#define TYPE_CMD_ERR  0x11
#define TYPE_FILE_ERR 0x12
#define TYPE_UKN_ERR  0x13
#define TYPE_DATA     0x20

#define CODE_NULL     0x00
#define CODE_OK       0x00
#define CODE_OK_SC    0x01
#define CODE_OK_CS    0x02
#define CODE_SYNTAXE  0x01
#define CODE_UKNCMD   0x02
#define CODE_PROTOCOL 0x03
#define CODE_NOTEX    0x00
#define CODE_DENIED   0x01
#define CODE_DEND     0x00
#define CODE_DCONT    0x01

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
extern void send_mypkt(int socket, struct myftph *pkt);
extern void send_mydata(int socket, struct myftph_data *pkt);
extern void print_hex(const unsigned char *data, int length);
extern int recv_myftp(int socket, struct myftph_data *pkt);

#endif  // MYFTP_TYPES_H_

