CC = gcc
CFLAGS = -O2 -Wall -g

OUTPUT := bufcache
SRCS := $(wildcard *.c)
objects := $(patsubst %.c,%.o,$(SRCS))
dependencies := $(patsubst %.c,%.d,$(SRCS))

.PHONY: all clean

all: $(OUTPUT)

$(OUTPUT): $(objects)
	$(CC) $(CFLAGS) $(objects) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -o $@ $<

-include $(dependencies)

clean:
	rm *.o *.d
	rm $(OUTPUT)

