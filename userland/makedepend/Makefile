# X.Org makedepend 1.0.10: host reference build for the DAIMOS port.
# The native PDP-6 executable is not yet supported; see PORT-STATUS.MD.
CC ?= cc
BUILD ?= build
CFLAGS ?= -O2
SRCS = main.c include.c parse.c pr.c cppsetup.c ifparser.c
OBJS = $(SRCS:.c=.o)
HOST_CFLAGS = -std=c99 -D_DEFAULT_SOURCE -DHAVE_RENAME=1 \
    -DOBJSUFFIX='".o"' -DINCLUDEDIR='"/usr/include"'

all: host
host:
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(HOST_CFLAGS) $(SRCS) -o $(BUILD)/makedepend

clean:
	rm -rf $(BUILD)

.PHONY: all host clean
