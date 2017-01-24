#include "types.h"

void
myftph_init(struct myftph *pkt, uint8_t type, uint8_t code)
{
  pkt->type = type;
  pkt->code = code;
  pkt->length = 0;
}

void
myftph_data_init(struct myftph_data *pkt, uint8_t type, uint8_t code)
{
  pkt->type = type;
  pkt->code = code;
  pkt->length = 0;
}

