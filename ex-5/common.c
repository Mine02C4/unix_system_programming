#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

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

void
send_mypkt(int socket, struct myftph *pkt)
{
  if (send(socket, pkt, sizeof(*pkt), 0) < 0) {
    fprintf(stderr, "Error in send: %s\n", strerror(errno));
    exit(1);
  }
}

void
send_mydata(int socket, struct myftph_data *pkt)
{
  size_t pkt_size = sizeof(struct myftph) + pkt->length;
  if (send(socket, pkt, pkt_size, 0) < 0) {
    fprintf(stderr, "Error in send: %s\n", strerror(errno));
    exit(1);
  }
}

void
send_byteseq(int socket, char *data, size_t length)
{
  MYFTPDATA(pkt, TYPE_DATA, CODE_DEND);
  size_t offset = 0;
  const size_t block_size = MAX_DATASIZE;
  for (; offset < length;) {
    size_t next_size;
    if (length - offset > block_size) {
      pkt.code = CODE_DCONT;
      next_size = block_size;
    } else {
      pkt.code = CODE_DEND;
      next_size = length - offset;
    }
    memcpy(pkt.data, data + offset, next_size);
    pkt.length = next_size;
    send_mydata(socket, &pkt);
    offset += next_size;
  }
}

int
recv_myftp(int socket, struct myftph_data *pkt)
{
  size_t recv_size;
  const size_t header_size = sizeof(struct myftph);
  if ((recv_size = recv(socket, pkt, header_size, 0)) < 0) {
    fprintf(stderr, "Error in recv: %s\n", strerror(errno));
    exit(errno);
  }
  if (recv_size != header_size) {
    fprintf(stderr, "Error: Invalid header size\n");
    return -1;
  }
  if (pkt->length > 0) {
    int c = 0;
    for (;;) {
      if ((recv_size = recv(socket, ((char*) pkt) + header_size + c, pkt->length - c, 0)) < 0) {
        fprintf(stderr, "Error in recv data: %s\n", strerror(errno));
        exit(errno);
      }
      c += recv_size;
      if (c == pkt->length) {
        break;
      } else if (c < pkt->length) {
        continue;
      } else {
        fprintf(stderr, "Error: oversize\n"); // unreachable
        return -1;
      }
    }
    printf("recv with data length = %d\n", c);
    print_hex((unsigned char *)pkt, header_size + c);
  }
  return 0;
}

void
print_hex(const unsigned char *data, int length)
{
  int i;
  const int width = 4;
  for (i = 0; i < length; i++) {
    printf("%02x ", data[i]);
    if (i % width == width - 1)
      putchar('\n');
  }
  putchar('\n');
}

char *
get_dirstr(DIR * dir, const char *dirname)
{
  const size_t buf_size = 1024;
  char *str = (char *)malloc(buf_size * 2);
  str[0] = '\0';
  size_t c_size = buf_size * 2;
  char buf[buf_size];
  char filename[1024];
  struct stat st;
  struct dirent *de;
  if (str == NULL) {
    fprintf(stderr, "Memory error!! malloc returns NULL!!\n");
    exit(1);
  }
  for (de = readdir(dir); de != NULL; de = readdir(dir)) {
    strcpy(filename, dirname);
    size_t baselen = strlen(filename);
    if (filename[baselen - 1] == '/') {
      strcat(filename, de->d_name);
    } else {
      filename[baselen] = '/';
      filename[baselen + 1] = '\0';
      strcat(filename, de->d_name);
    }
    if (stat(filename, &st) < 0) {
      continue;
    } else {
      get_filestr(buf, buf_size, &st, de->d_name);
      strcat(str, buf);
      if (c_size - strlen(str) - 1 < buf_size) {
        c_size += buf_size;
        char *str_t = (char *)malloc(c_size);
        if (str_t == NULL) {
          fprintf(stderr, "Memory error!! malloc returns NULL!!\n");
          exit(1);
        }
        strcpy(str_t, str);
        free(str);
        str = str_t;
      }
    }
  }
  return str;
}

void
get_filestr(char *buf, size_t buf_size, struct stat *st, const char *name)
{
  if (S_ISDIR(st->st_mode)) {
    buf[0] = 'd';
  } else if (S_ISLNK(st->st_mode)) {
    buf[0] = 'l';
  } else {
    buf[0] = '-';
  }
  if (st->st_mode & S_IRUSR) {
    buf[1] = 'r';
  } else {
    buf[1] = '-';
  }
  if (st->st_mode & S_IWUSR) {
    buf[2] = 'w';
  } else {
    buf[2] = '-';
  }
  if (st->st_mode & S_IXUSR) {
    buf[3] = 'x';
  } else {
    buf[3] = '-';
  }
  if (st->st_mode & S_IRGRP) {
    buf[4] = 'r';
  } else {
    buf[4] = '-';
  }
  if (st->st_mode & S_IWGRP) {
    buf[5] = 'w';
  } else {
    buf[5] = '-';
  }
  if (st->st_mode & S_IXGRP) {
    buf[6] = 'x';
  } else {
    buf[6] = '-';
  }
  if (st->st_mode & S_IROTH) {
    buf[7] = 'r';
  } else {
    buf[7] = '-';
  }
  if (st->st_mode & S_IWOTH) {
    buf[8] = 'w';
  } else {
    buf[8] = '-';
  }
  if (st->st_mode & S_IXOTH) {
    buf[9] = 'x';
  } else {
    buf[9] = '-';
  }
  buf[10] = ' ';
  snprintf(buf + 11, buf_size - 11, "%10ld %s\n", st->st_size, name);
}

