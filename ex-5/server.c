#include "server.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int s;

void
setup_socket()
{
  in_port_t myport = MYFTP_SERVER_PORT;
  struct sockaddr_in myskt;
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
    exit(errno);
  }
  memset(&myskt, 0, sizeof(myskt));
  myskt.sin_family = AF_INET;
  myskt.sin_port = htons(myport);
  myskt.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr *) &myskt, sizeof(myskt)) < 0) {
    fprintf(stderr, "Error in binding: %s\n", strerror(errno));
    exit(errno);
  }
  if (listen(s, 5) < 0) {
    fprintf(stderr, "Error in listen: %s\n", strerror(errno));
    exit(errno);
  }
}

void
force_disconnect(int sd)
{
  close(sd);
  exit(1);
}

void
wait_connect(struct sockaddr_in *skt, int *socket)
{
  int s2;
  socklen_t sktlen = sizeof(*skt);
  if ((s2 = accept(s, (struct sockaddr *)skt, &sktlen)) < 0) {
    fprintf(stderr, "Error in accept: %s\n", strerror(errno));
    exit(errno);
  }
  *socket = s2;
  printf("Connected from: %s: %d\n", inet_ntoa(skt->sin_addr), ntohs(skt->sin_port));
}

void
start_server(int sd)
{
  printf("Start server\n");
  struct myftph_data pkt;
  size_t header_size = sizeof(struct myftph);
  size_t recv_size;
  for (;;) {
    if ((recv_size = recv(sd, &pkt, header_size, 0)) < 0) {
      fprintf(stderr, "Error in recv: %s\n", strerror(errno));
      exit(errno);
    }
    if (recv_size == 0) {
      fprintf(stderr, "Connection refused.\n");
      exit(1);
    }
    if (recv_size != header_size) {
      fprintf(stderr, "Error: Invalid header size = %ld\n", recv_size);
      force_disconnect(sd);
      continue;
    }
    if (pkt.length > 0) {
      int c = 0;
      for (;;) {
        if ((recv_size = recv(sd, ((void*) &pkt) + header_size + c, pkt.length - c, 0)) < 0) {
          fprintf(stderr, "Error in recv data: %s\n", strerror(errno));
          exit(errno);
        }
        if (recv_size == 0) {
          fprintf(stderr, "Connection refused.\n");
          exit(1);
        }
        c += recv_size;
        if (c == pkt.length) {
          break;
        } else if (c < pkt.length) {
          continue;
        } else {
          fprintf(stderr, "Error: oversize\n"); // unreachable
          force_disconnect(sd);
        }
      }
    }
    switch (pkt.type) {
      case TYPE_QUIT:
        {
          printf("QUIT\n");
          close(sd);
          exit(0);
          break;
        }
      case TYPE_PWD:
        {
          printf("PWD\n");
          MYFTPDATA(pkt, TYPE_OK, CODE_OK);
          if (getcwd(pkt.data, MAX_DATASIZE) == NULL) {
            fprintf(stderr, "Error in getcwd: %s\n", strerror(errno));
            force_disconnect(sd);
          }
          pkt.length = strlen(pkt.data);
          printf("strlen = %ld\n dir = %s\n", strlen(pkt.data), pkt.data);
          print_hex((unsigned char *)&pkt, sizeof(struct myftph) + pkt.length);
          send_mydata(sd, &pkt);
          break;
        }
      case TYPE_CWD:
        {
          pkt.data[pkt.length] = '\0';
          printf("CWD '%s'\n", pkt.data);
          if (chdir(pkt.data) < 0) {
            MYFTPPKT(rpkt, TYPE_FILE_ERR, CODE_NULL);
            if (errno == EACCES) {
              rpkt.code = CODE_DENIED;
            } else {
              rpkt.code = CODE_NOTEX;
            }
            send_mypkt(sd, &rpkt);
          } else {
            MYFTPPKT(rpkt, TYPE_OK, CODE_OK);
            send_mypkt(sd, &rpkt);
          }
          break;
        }
      case TYPE_LIST:
        {
          DIR *dir;
          char *dirname;
          if (pkt.length == 0) {
            printf("LIST\n");
            dir = opendir(".");
            dirname = ".";
          } else {
            pkt.data[pkt.length] = '\0';
            printf("LIST '%s'\n", pkt.data);
            struct stat st;
            if (stat(pkt.data, &st) < 0) {
              MYFTPPKT(rpkt, TYPE_FILE_ERR, CODE_NULL);
              if (errno == EACCES) {
                rpkt.code = CODE_DENIED;
              } else {
                rpkt.code = CODE_NOTEX;
              }
              send_mypkt(sd, &rpkt);
              break;
            }
            if (S_ISDIR(st.st_mode)) {
              dir = opendir(pkt.data);
              dirname = pkt.data;
            } else {
              MYFTPPKT(rpkt, TYPE_OK, CODE_OK_SC);
              send_mypkt(sd, &rpkt);
              MYFTPDATA(dpkt, TYPE_DATA, CODE_DEND);
              get_filestr(dpkt.data, MAX_DATASIZE, &st, pkt.data);
              dpkt.length = strlen(dpkt.data);
              send_mydata(sd, &dpkt);
              break;
            }
          }
          if (dir == NULL) {
            MYFTPPKT(rpkt, TYPE_FILE_ERR, CODE_NULL);
            if (errno == EACCES) {
              rpkt.code = CODE_DENIED;
            } else {
              rpkt.code = CODE_NOTEX;
            }
            send_mypkt(sd, &rpkt);
          } else {
            char *str_data = get_dirstr(dir, dirname);
            closedir(dir);
            MYFTPPKT(rpkt, TYPE_OK, CODE_OK_SC);
            send_mypkt(sd, &rpkt);
            send_byteseq(sd, str_data, strlen(str_data));
            free(str_data);
          }
          break;
        }
      case TYPE_RETR:
        {
          if (pkt.length == 0) {
            MYFTPPKT(rpkt, TYPE_CMD_ERR, CODE_SYNTAXE);
            send_mypkt(sd, &rpkt);
          } else {
            int fd;
            pkt.data[pkt.length] = '\0';
            printf("RETR '%s'\n", pkt.data);
            if ((fd = open(pkt.data, O_RDONLY)) < 0) {
              MYFTPPKT(rpkt, TYPE_FILE_ERR, CODE_NULL);
              if (errno == EACCES) {
                rpkt.code = CODE_DENIED;
              } else {
                rpkt.code = CODE_NOTEX;
              }
              send_mypkt(sd, &rpkt);
              break;
            }
            MYFTPPKT(rpkt, TYPE_OK, CODE_OK_SC);
            send_mypkt(sd, &rpkt);
            MYFTPDATA(dpkt, TYPE_DATA, CODE_DCONT);
            size_t rsize;
            for (;;) {
              if ((rsize = read(fd, dpkt.data, MAX_DATASIZE)) < 0) {
                fprintf(stderr, "Error in read: %s\n", strerror(errno));
                force_disconnect(sd);
              }
              if (rsize < MAX_DATASIZE) {
                dpkt.code = CODE_DEND;
              }
              dpkt.length = rsize;
              send_mydata(sd, &dpkt);
              if (rsize < MAX_DATASIZE) {
                break;
              }
            }
            close(fd);
          }
          break;
        }
      case TYPE_STOR:
        {
          if (pkt.length == 0) {
            MYFTPPKT(rpkt, TYPE_CMD_ERR, CODE_SYNTAXE);
            send_mypkt(sd, &rpkt);
          } else {
            int fd;
            pkt.data[pkt.length] = '\0';
            printf("STOR '%s'\n", pkt.data);
            if ((fd = open(pkt.data, O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0) {
              MYFTPPKT(rpkt, TYPE_FILE_ERR, CODE_NULL);
              if (errno == EACCES) {
                rpkt.code = CODE_DENIED;
              } else {
                rpkt.code = CODE_NOTEX;
              }
              send_mypkt(sd, &rpkt);
              break;
            }
            MYFTPPKT(rpkt, TYPE_OK, CODE_OK_CS);
            send_mypkt(sd, &rpkt);
            struct myftph_data dpkt;
            for (;;) {
              if (recv_myftp(sd, &dpkt) < 0) {
                force_disconnect(sd);
                break;
              }
              if (write(fd, dpkt.data, dpkt.length) < 0) {
                fprintf(stderr, "Error in write: %s\n", strerror(errno));
                force_disconnect(sd);
              }
              if (dpkt.code == CODE_DEND) {
                break;
              }
            }
            close(fd);
          }
          break;
        }
      default:
        {
          fprintf(stderr, "Unknown type\n");
          MYFTPPKT(rpkt, TYPE_CMD_ERR, CODE_UKNCMD);
          send_mypkt(sd, &rpkt);
          break;
        }
    }
  }
}

int
main(const int argc, const char *argv[])
{
  if (argc > 2) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stderr, "Usase: ./myftpd [<working-directory>]\n");
    exit(1);
  }
  if (argc == 2) {
    if (chdir(argv[1]) < 0) {
      fprintf(stderr, "Error in chdir: %s\n", strerror(errno));
      exit(errno);
    }
  }
  setup_socket();
  for (;;) {
    struct sockaddr_in skt;
    int s2;
    wait_connect(&skt, &s2);
    int pid = fork();
    if (pid < 0) {
      fprintf(stderr, "Error in fork: %s\n", strerror(errno));
      exit(errno);
    } else if (pid == 0) { // Child
      close(s);
      start_server(s2);
      close(s2);
      break;
    } else { // Parent
      close(s2);
    }
  }
  return 0;
}


