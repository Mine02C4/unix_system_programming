#include <stddef.h>

#include "buffer.h"

static struct buf_header hash_head[NHASH];
static struct buf_header free_head;

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

int
free_is_empty()
{
  return free_head.free_fp == &free_head;
}

struct buf_header *
get_first_free()
{
  return free_head.free_fp;
}

void
remove_buffer_from_free_list(struct buf_header *p)
{
  p->free_bp->free_fp = p->free_fp;
  p->free_fp->free_bp = p->free_bp;
}

void
enqueue_buffer_at_head(struct buf_header *p)
{
  p->free_bp = &free_head;
  p->free_fp = free_head.free_fp;
  p->free_fp->free_bp = p;
  p->free_bp->free_fp = p;
}

void
enqueue_buffer_at_end(struct buf_header *p)
{
  p->free_fp = &free_head;
  p->free_bp = free_head.free_bp;
  p->free_fp->free_bp = p;
  p->free_bp->free_fp = p;
}

void
insert_buffer(struct buf_header *p)
{
  int h = hash(p->blkno);
  insert_bottom(&hash_head[h], p);
}
