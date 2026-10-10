# DAIMOS PDP-10 C runtime library. Compatible with GNU make and BSD make.
PDP10_PREFIX ?=
LIBC_ROOT != pwd
BUILD_ROOT ?= $(LIBC_ROOT)/../build
BUILD = $(BUILD_ROOT)/libc
CC ?= kcc
DAS = das
DASFLAGS ?= -F
DARC = darc
CFLAGS = -Pgnu99 -O -x=pdp6 -m=gas
CPPFLAGS = -I$(LIBC_ROOT) -I$(LIBC_ROOT)/../../system/kernel/core \
	-I$(LIBC_ROOT)/../../system/kernel/fs \
	-I$(LIBC_ROOT)/../../system/kernel/proc \
	-I$(LIBC_ROOT)/../../system/kernel/storage \
	-I$(PDP10_PREFIX)/include

ASM_SRCS = memcpy.s memmove.s syscall.s
C_SRCS = stdlib.c string.c stat_time.c process_ctype.c u.c
ASM_OBJS = $(BUILD)/memcpy.dobj $(BUILD)/memmove.dobj $(BUILD)/syscall.dobj
C_OBJS = $(BUILD)/stdlib.dobj $(BUILD)/string.dobj \
	$(BUILD)/stat_time.dobj $(BUILD)/process_ctype.dobj $(BUILD)/u.dobj
OBJS = $(ASM_OBJS) $(C_OBJS)
LIBC = $(BUILD)/libc.a

.PHONY: build install clean check-prefix
build: check-prefix $(LIBC)

check-prefix:
	@test -n "$(PDP10_PREFIX)" || { echo "PDP10_PREFIX must be set" >&2; exit 1; }
	@command -v "$(CC)" >/dev/null 2>&1 || { echo "missing compiler: $(CC)" >&2; exit 1; }
	@command -v "$(DAS)" >/dev/null 2>&1 || { echo "missing assembler: $(DAS)" >&2; exit 1; }
	@command -v "$(DARC)" >/dev/null 2>&1 || { echo "missing archiver: $(DARC)" >&2; exit 1; }

# Explicit dependencies avoid dialect-specific pattern rules. Shell filename
# expansion lets the shared recipes select their individual source files.
$(ASM_OBJS):
	@mkdir -p "$(BUILD)"
	@src="$@"; src=$${src##*/}; src=$${src%.dobj}.s; \
		"$(DAS)" $(DASFLAGS) -C -O "$@" "$$src"

$(C_OBJS):
	@mkdir -p "$(BUILD)"
	@src="$@"; src=$${src##*/}; src=$${src%.dobj}; \
		"$(CC)" $(CFLAGS) $(CPPFLAGS) -S "$$src.c" -o "$(BUILD)/$$src.s"
	@src="$@"; src=$${src%.dobj}.s; \
		"$(DAS)" $(DASFLAGS) -C -O "$@" "$$src"

$(BUILD)/memcpy.dobj: memcpy.s
$(BUILD)/memmove.dobj: memmove.s
$(BUILD)/syscall.dobj: syscall.s
$(BUILD)/stdlib.dobj: stdlib.c
$(BUILD)/string.dobj: string.c
$(BUILD)/stat_time.dobj: stat_time.c
$(BUILD)/process_ctype.dobj: process_ctype.c
$(BUILD)/u.dobj: u.c

$(LIBC): $(OBJS)
	"$(DARC)" -o "$@" $(OBJS)

install: build
	mkdir -p "$(PDP10_PREFIX)/lib"
	cp "$(LIBC)" "$(PDP10_PREFIX)/lib/libc.a"

clean:
	rm -rf "$(BUILD)"
