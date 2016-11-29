CC = gcc
CFLAGS = -O2 -Wall -g

OUTPUT := mysh
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
	$(RM) *.o *.d $(OUTPUT)

