#include "client.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

#include "types.h"

#define MAX_LINE 1024

void getargs(const char *input, int max, int *argc, char *argv[]);

typedef void (* cmdfuncptr)(int, char**);

void quitcmd(int, char**);
void pwdcmd(int, char**);
void cdcmd(int, char**);
void dircmd(int, char**);
void lpwdcmd(int, char**);
void lcdcmd(int, char**);
void ldircmd(int, char**);
void getcmd(int, char**);
void putcmd(int, char**);

struct cmdinfo {
  char *name;
  cmdfuncptr func;
};

const struct cmdinfo cmdarray[] = {
  {"quit",    quitcmd},
  {"pwd",     pwdcmd},
  {"cd",      cdcmd},
  {"dir",     dircmd},
  {"lpwd",    lpwdcmd},
  {"lcd",     lcdcmd},
  {"ldir",    ldircmd},
  {"get",     getcmd},
  {"put",     putcmd},
};

void
cmdline()
{
  char line[MAX_LINE];
  for (;;) {
    printf("myFTP%% ");
    fflush(stdout);
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      fprintf(stderr, "Input error when fgets.\n");
      exit(1);
    }
    int argc;
    char *argv[MAX_LINE];
    getargs(line, MAX_LINE, &argc, argv);
    if (argc > 0 && argv[0][0] != '\0') {
      int i;
      int validcmd = 0;
      for (i = 0; i < sizeof(cmdarray) / sizeof(cmdarray[0]); i++) {
        if (strcmp(cmdarray[i].name, argv[0]) == 0) {
          validcmd = 1;
          cmdarray[i].func(argc, argv);
          break;
        }
      }
      if (!validcmd) {
        fprintf(stderr, "Unkown command '%s'.\n", argv[0]);
      }
    }
  }
}

int s;

void
error_exit()
{
  fprintf(stderr, "Exit by error\n");
  close(s);
  exit(1);
}

void
quitcmd(int argc, char** argv)
{
  MYFTPPKT(pkt, TYPE_QUIT, CODE_NULL);
  print_hex((unsigned char *)&pkt, sizeof(pkt));
  send_mypkt(s, &pkt);
  close(s);
  exit(0);
}

void
pwdcmd(int argc, char** argv)
{
  MYFTPPKT(pkt, TYPE_PWD, CODE_NULL);
  print_hex((unsigned char *)&pkt, sizeof(pkt));
  send_mypkt(s, &pkt);
  struct myftph_data rpkt;
  if (recv_myftp(s, &rpkt) < 0) {
    error_exit();
  }
  rpkt.data[rpkt.length] = '\0';
  printf("%s\n", rpkt.data);
}

void
cdcmd(int argc, char** argv)
{
  if (argc == 2) {
    MYFTPDATA(pkt, TYPE_CWD, CODE_NULL);
    int len = strlen(argv[1]);
    strncpy(pkt.data, argv[1], len);
    pkt.length = len;
    send_mydata(s, &pkt);
    struct myftph_data rpkt;
    if (recv_myftp(s, &rpkt) < 0) {
      error_exit();
    }
    if (rpkt.type == TYPE_OK) {
      printf("OK\n");
    } else if (rpkt.type == TYPE_FILE_ERR) {
      if (rpkt.code == CODE_NOTEX) {
        fprintf(stderr, "Not found\n");
      } else if (rpkt.code == CODE_DENIED) {
        fprintf(stderr, "Access denied\n");
      } else {
        fprintf(stderr, "Invalid error code = %d\n", rpkt.code);
      }
    } else {
      fprintf(stderr, "Invalid type\n");
    }
  } else {
    fprintf(stderr, "Invalid argument: Command '%s' requires 1 parameter.\n", argv[0]);
  }
}
void
dircmd(int argc, char** argv)
{
  if (argc == 1 || argc == 2) {
    MYFTPDATA(pkt, TYPE_LIST, CODE_NULL);
    if (argc == 2) {
      int len = strlen(argv[1]);
      strncpy(pkt.data, argv[1], len);
      pkt.length = len;
    }
    send_mydata(s, &pkt);
    struct myftph_data rpkt;
    if (recv_myftp(s, &rpkt) < 0) {
      error_exit();
    }
    rpkt.data[rpkt.length] = '\0';
    printf("%s\n", rpkt.data);
    if (rpkt.type == TYPE_OK) {
      if (rpkt.code == CODE_OK_SC) {
        struct myftph_data dpkt;
        for (;;) {
          if (recv_myftp(s, &dpkt) < 0) {
            error_exit();
            break;
          }
          char buf[MAX_DATASIZE + 1];
          memcpy(buf, dpkt.data, dpkt.length);
          buf[dpkt.length] = '\0';
          printf("%s", buf);
          if (dpkt.code == CODE_DEND) {
            break;
          }
        }
      } else {
        printf("OK but no data\n");
      }
    } else if (rpkt.type == TYPE_FILE_ERR) {
      if (rpkt.code == CODE_NOTEX) {
        fprintf(stderr, "Not found\n");
      } else if (rpkt.code == CODE_DENIED) {
        fprintf(stderr, "Access denied\n");
      } else {
        fprintf(stderr, "Invalid error code = %d\n", rpkt.code);
      }
    } else {
      fprintf(stderr, "Invalid type\n");
    }
  } else {
    fprintf(stderr, "Invalid argument: Command '%s' requires 0 or 1 parameter.\n", argv[0]);
  }
}

void
lpwdcmd(int argc, char** argv)
{
  char buf[1024];
  if (getcwd(buf, 1024) == NULL) {
    fprintf(stderr, "Error in getcwd: %s\n", strerror(errno));
    return;
  }
  printf("%s\n", buf);
}

void
lcdcmd(int argc, char** argv)
{
  if (argc == 2) {
    if (chdir(argv[1]) < 0) {
      fprintf(stderr, "Error in chdir: %s\n", strerror(errno));
    }
  } else {
    fprintf(stderr, "Invalid argument: Command '%s' requires 1 parameter.\n", argv[0]);
  }
}

void
ldircmd(int argc, char** argv)
{
  if (argc == 1) {
    DIR *dir = opendir(".");
    char *str_data = get_dirstr(dir, ".");
    printf("%s\n", str_data);
    closedir(dir);
    free(str_data);
  }
}

void
getcmd(int argc, char** argv)
{}

void
putcmd(int argc, char** argv)
{}

void
setup_socket(const char *hostname)
{
  struct addrinfo hints, *res;
  int err;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  char strport[50];
  snprintf(strport, 50, "%d", MYFTP_SERVER_PORT);
  if ((err = getaddrinfo(hostname, strport, &hints, &res)) < 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    exit(1);
  }
  if ((s = socket(res->ai_family, res->ai_socktype, 0)) < 0) {
    fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
    exit(errno);
  }
  printf("Server: %s: %d\n",
      inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr),
      ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port));
  if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
    fprintf(stderr, "Error in connect: %s\n", strerror(errno));
    exit(errno);
  }
  freeaddrinfo(res);
}

int
main(const int argc, const char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Invalid arguments.\n");
    fprintf(stderr, "Usase: ./myftpc <server-name>\n");
    exit(1);
  }
  setup_socket(argv[1]);
  cmdline();
  close(s);
  return 0;
}

