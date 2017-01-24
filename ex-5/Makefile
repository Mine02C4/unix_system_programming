CC = gcc
CFLAGS = -O2 -Wall -g

CLIENT := myftpc
SERVER := myftpd
SRCS := $(wildcard *.c)
DEPS := $(patsubst %.c,%.d,$(SRCS))

.PHONY: all clean

all: $(CLIENT) $(SERVER)

$(CLIENT): client.o common.o getargs.o
	$(CC) $(CFLAGS) $^ -o $@

$(SERVER): server.o common.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -o $@ $<

-include $(DEPS)

clean:
	$(RM) *.o *.d $(CLIENT) $(SERVER)

