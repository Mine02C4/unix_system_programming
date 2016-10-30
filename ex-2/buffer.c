#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

static struct buf_header hash_head[NHASH];
static struct buf_header free_head;
static struct buf_header *buf_area = NULL;
int buffer_size = 0;

inline static int
hash(int blkno)
{
  return blkno % NHASH;
}

void
init_head()
{
  int i;
  for (i = 0; i < NHASH; i++) {
    hash_head[i].hash_fp = &hash_head[i];
    hash_head[i].hash_bp = &hash_head[i];
  }
  free_head.free_fp = &free_head;
  free_head.free_bp = &free_head;
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

void
init_buffer()
{
  if (buf_area != NULL)
    free(buf_area);
  init_head();
  const int inserts[] = {28, 4, 64, 17, 5, 97, 98, 50, 10, 3, 35, 99};
  const int insert_size = sizeof(inserts) / sizeof(int);
  const int free_array[] = {3, 5, 4, 28, 97, 10};
  const int free_size = sizeof(free_array) / sizeof(int);
  int i;
  if ((buf_area = (struct buf_header *)malloc(sizeof(struct buf_header) * insert_size)) == NULL) {
    fprintf(stderr, "Memory error!! malloc returns NULL!!\n");
    exit(1);
  }
  buffer_size = insert_size;
  for (i = 0; i < insert_size; i++) {
    struct buf_header *p = &buf_area[i];
    p->blkno = inserts[i];
    p->stat = STAT_VALID | STAT_LOCKED;
    insert_buffer(p);
  }
  for (i = 0; i < free_size; i++) {
    struct buf_header *p;
    p = hash_search(free_array[i]);
    p->stat &= ~STAT_LOCKED;
    enqueue_buffer_at_end(p);
  }
}

void
print_buf_header(const struct buf_header *p)
{
  printf("[%2d:%3d %c%c%c%c%c%c]", p - buf_area, p->blkno,
      p->stat & STAT_OLD     ? 'O' : '-',
      p->stat & STAT_WAITED  ? 'W' : '-',
      p->stat & STAT_KRDWR   ? 'K' : '-',
      p->stat & STAT_DWR     ? 'D' : '-',
      p->stat & STAT_VALID   ? 'V' : '-',
      p->stat & STAT_LOCKED  ? 'L' : '-'
  );
}

void
print_buffer(int number)
{
  print_buf_header(&buf_area[number]);
  putchar('\n');
}

void
print_hash(int h)
{
  printf("%d: ", h);
  struct buf_header *p;
  for (p = hash_head[h].hash_fp; p != &hash_head[h]; p = p->hash_fp) {
    print_buf_header(p);
    if (p != &free_head)
      putchar(' ');
  }
  putchar('\n');
}

void
print_free()
{
  struct buf_header *p;
  for (p = free_head.free_fp; p != &free_head; p = p->free_fp) {
    print_buf_header(p);
    if (p != &free_head)
      putchar(' ');
  }
  putchar('\n');
}

