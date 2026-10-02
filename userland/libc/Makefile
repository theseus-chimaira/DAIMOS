PDP10_PREFIX ?=
BUILD_ROOT ?= $(CURDIR)/../build
BUILD := $(BUILD_ROOT)/libc
CC := $(PDP10_PREFIX)/bin/kcc
DAS := $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -F
DARC := $(PDP10_PREFIX)/bin/darc
CFLAGS := -Pgnu99 -O -x=pdp6 -m=gas
OBJS := $(BUILD)/memcpy.dobj $(BUILD)/memmove.dobj
LIBC := $(BUILD)/libc.a

.PHONY: build install clean check-prefix

build: check-prefix $(LIBC)

check-prefix:
	@test -n "$(PDP10_PREFIX)" || { echo "PDP10_PREFIX must be set" >&2; exit 1; }
	@test -x "$(CC)" || { echo "missing compiler: $(CC)" >&2; exit 1; }
	@test -x "$(DAS)" || { echo "missing assembler: $(DAS)" >&2; exit 1; }
	@test -x "$(DARC)" || { echo "missing archiver: $(DARC)" >&2; exit 1; }

$(BUILD):
	mkdir -p "$@"

$(BUILD)/%.dobj: %.s | $(BUILD)
	"$(DAS)" $(DASFLAGS) -C -O "$@" "$<"

$(BUILD)/%.s: %.c $(CC) | $(BUILD)
	"$(CC)" $(CFLAGS) -S "$<" -o "$@"

$(BUILD)/%.dobj: $(BUILD)/%.s
	"$(DAS)" $(DASFLAGS) -C -O "$@" "$<"

$(LIBC): $(OBJS)
	"$(DARC)" -o "$@" $(OBJS)

install: build
	mkdir -p "$(PDP10_PREFIX)/lib"
	cp "$(LIBC)" "$(PDP10_PREFIX)/lib/libc.a"

clean:
	rm -rf "$(BUILD)"
