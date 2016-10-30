#include <stdio.h>

#include "buffer.h"

struct buf_header *
getblk(int blkno)
{
  while (1) {
    struct buf_header *p;
    if ((p = hash_search(blkno)) != NULL) {
      if (p->stat & STAT_LOCKED) {
        /* Scenario 5 */
        printf("Scenario 5\n");
        p->stat |= STAT_WAITED;
        // sleep(event buffer becomes free);
        printf("Process goes to sleep\n");
        return NULL;
      }
      /* Scenario 1 */
      printf("Scenario 1\n");
      p->stat |= STAT_LOCKED;
      remove_buffer_from_free_list(p);
      return p;
    } else {
      if (free_is_empty()) {
        /* Scenario 4 */
        printf("Scenario 4\n");
        p->stat |= STAT_WAITED;
        // sleep(event any buffer becomes free);
        printf("Process goes to sleep\n");
        return NULL;
      }
      p = get_first_free();
      remove_buffer_from_free_list(p);
      p->stat |= STAT_LOCKED;
      if (p->stat & STAT_DWR) {
        /* Scenario 3 */
        printf("Scenario 3\n");
        p->stat |= STAT_OLD;
        // asynchronous write buffer to disk;
        continue;
      }
      /* Scenario 2 */
      printf("Scenario 2\n");
      remove_hash(p);
      p->blkno = blkno;
      p->stat &= ~STAT_VALID;
      insert_buffer(p);
      return p;
    }
  }
}

