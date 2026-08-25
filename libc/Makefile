PDP10_PREFIX ?=
BUILD_ROOT ?= $(CURDIR)/../build
BUILD := $(BUILD_ROOT)/libc
CC := $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
AS := $(PDP10_PREFIX)/bin/pdp10-dec-none-as
DARC := $(PDP10_PREFIX)/bin/darc
CFLAGS := -std=c99 -Os -fno-builtin -fno-common
OBJS := $(BUILD)/memcpy.dobj $(BUILD)/memmove.dobj
LIBC := $(BUILD)/libc.a

.PHONY: build install clean check-prefix

build: check-prefix $(LIBC)

check-prefix:
	@test -n "$(PDP10_PREFIX)" || { echo "PDP10_PREFIX must be set" >&2; exit 1; }
	@test -x "$(CC)" || { echo "missing compiler: $(CC)" >&2; exit 1; }
	@test -x "$(AS)" || { echo "missing assembler: $(AS)" >&2; exit 1; }
	@test -x "$(DARC)" || { echo "missing archiver: $(DARC)" >&2; exit 1; }

$(BUILD):
	mkdir -p "$@"

$(BUILD)/%.s: %.c | $(BUILD)
	"$(CC)" $(CFLAGS) -S "$<" -o "$@"

$(BUILD)/%.dobj: $(BUILD)/%.s
	"$(AS)" -c -o "$@" "$<"

$(LIBC): $(OBJS)
	"$(DARC)" -o "$@" $(OBJS)

install: build
	mkdir -p "$(PDP10_PREFIX)/lib"
	cp "$(LIBC)" "$(PDP10_PREFIX)/lib/libc.a"

clean:
	rm -rf "$(BUILD)"
