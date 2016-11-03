#include <stdio.h>

#include "buffer.h"

void
brelse(struct buf_header *buffer)
{
  // wakeup();
  printf("Wakeup processes waiting for any buffer\n");
  if (buffer->stat & STAT_WAITED) {
    // wakeup();
    printf("Wakeup processes waiting for buffer of blkno %d\n", buffer->blkno);
    buffer->stat &= ~STAT_WAITED;
  }
  // raise_cpu_level();
  if (buffer->stat & STAT_VALID && !(buffer->stat & STAT_OLD)) {
    enqueue_buffer_at_end(buffer);
  } else {
    enqueue_buffer_at_head(buffer);
    buffer->stat &= ~STAT_OLD;
  }
  // lower_cpu_level();
  buffer->stat &= ~STAT_LOCKED;
}

