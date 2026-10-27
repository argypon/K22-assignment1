CC ?= cc
CFLAGS ?= -O2
CFLAGS += -Wall -Wextra
KERNEL_HEADERS ?= /usr/include
CPPFLAGS += -I$(KERNEL_HEADERS)

.PHONY: all clean

all: k22test

k22test: k22test.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<

clean:
	$(RM) k22test
