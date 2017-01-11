CC = gcc
CFLAGS = -O2 -Wall -g

CLIENT := mydhcpc
SERVER := mydhcpd
SRCS := $(wildcard *.c)
#objects := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.c,%.d,$(SRCS))

.PHONY: all clean

all: $(CLIENT) $(SERVER)

$(CLIENT): client.o
	$(CC) $(CFLAGS) $^ -o $@

$(SERVER): server.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -o $@ $<

-include $(DEPS)

clean:
	$(RM) *.o *.d $(CLIENT) $(SERVER)

