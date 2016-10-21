#include <stddef.h>

#include "buffer.h"

struct buf_header hash_head[NHASH];

inline int
hash(int blkno)
{
  return blkno % NHASH;
}

struct buf_header *
hash_search(int blkno)
{
  int h;
  struct buf_header *p;

  h = hash(blkno);
  for (p = hash_head[h].hash_fp; p != &hash_head[h]; p = p->hash_fp) {
    if (p->blkno == blkno)
      return p;
  }
  return NULL;
}

void
insert_head(struct buf_header *h, struct buf_header *p)
{
  p->hash_bp = h;
  p->hash_fp = h->hash_fp;
  p->hash_fp->hash_bp = p;
  p->hash_bp->hash_fp = p;
}

void
insert_bottom(struct buf_header *h, struct buf_header *p)
{
  p->hash_fp = h;
  p->hash_bp = h->hash_bp;
  p->hash_fp->hash_bp = p;
  p->hash_bp->hash_fp = p;
}

void
remove_hash(struct buf_header *p)
{
  p->hash_bp->hash_fp = p->hash_fp;
  p->hash_fp->hash_bp = p->hash_bp;
}

