/* 61414050 丹羽直也 */

#include <stdio.h>

#define NNODE 6
#define INF 100 // infinity

int cost[NNODE][NNODE] = {
  { 0,  2,   5, 1,   INF, INF},
  { 2,  0,   3, 2,   INF, INF},
  { 5,  3,   0, 3,   1,   5},
  { 1,  2,   3, 0,   1,   INF},
  {INF, INF, 1, 1,   0,   2},
  {INF, INF, 5, INF, 2,   0}
};

int dist[NNODE];
int prev[NNODE];
int conf[NNODE];

void init()
{
  int i;
  for (i = 0; i < NNODE; i++) {
    dist[i] = INF;
    conf[i] = 0;
  }
}

void print_result(int root_index)
{
  printf("root node %c\n", 'A' + root_index);
  printf("    ");
  int i;
  for (i = 0; i < NNODE; i++) {
    printf("[%c,%c,%d]", 'A' + i, 'A' + prev[i], dist[i]);
  }
  printf("\n");
}

void spf(int start_index) {
  int pos = start_index;
  int d = 0;
  int remain = NNODE;
  dist[pos] = 0;
  while (1) {
    conf[pos] = 1;
    remain--;
    int mind = INF;
    int minpos;
    int i;
    for (i = 0; i < NNODE; i++) {
      if (conf[i] != 0) {
        continue;
      }
      if (i != pos && cost[pos][i] < INF) {
        int td = d + cost[pos][i];
        if (td < dist[i]) {
          dist[i] = td;
          prev[i] = pos;
        }
      }
      if (dist[i] < mind) {
        mind = dist[i];
        minpos = i;
      }
    }
    if (remain == 0) {
      break;
    }
    pos = minpos;
    d = mind;
  }
  print_result(start_index);
}

int main()
{
  int i;
  for (i = 0; i < NNODE; i++) {
    init();
    spf(i);
  }
  return 0;
}

