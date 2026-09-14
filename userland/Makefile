PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
USERLAND_BUILD := $(BUILD_ROOT)/userland
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
DAS ?= $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -O
DLINK ?= $(PDP10_PREFIX)/bin/dlink
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel/boot -I$(PROJECT_ROOT)/system/kernel/core -I$(PROJECT_ROOT)/system/kernel/drivers -I$(PROJECT_ROOT)/system/kernel/fs -I$(PROJECT_ROOT)/system/kernel/mm -I$(PROJECT_ROOT)/system/kernel/modules -I$(PROJECT_ROOT)/system/kernel/proc -I$(PROJECT_ROOT)/system/kernel/storage -I$(PROJECT_ROOT)/userland/libc -I$(PROJECT_ROOT)/userland/exec
CFLAGS = -std=c99 -Os

CHECK_SRCS = libc/u.c exec/commands.c exec/standalone.c dsh/dsh.c
CHECK_ASMS = $(CHECK_SRCS:%.c=$(USERLAND_BUILD)/check/%.s)
DSH_C_SRCS = libc/u.c exec/commands.c dsh/dsh.c
DSH_C_OBJS = $(DSH_C_SRCS:%.c=$(USERLAND_BUILD)/dsh/%.dobj)
DSH_S_SRCS = libc/crt0.s libc/syscall_helpers.s
DSH_S_OBJS = $(DSH_S_SRCS:%.s=$(USERLAND_BUILD)/dsh/%.dobj)
DSH_DXR = $(USERLAND_BUILD)/dsh.dxr
DSH_MAP = $(USERLAND_BUILD)/dsh.map

.DEFAULT_GOAL := build
.PHONY: build check clean

build: $(DSH_DXR)

check: $(CHECK_ASMS) $(DSH_DXR)

$(USERLAND_BUILD)/check/%.s: %.c
	mkdir -p $(dir $@)
	$(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -S $< -o $@

$(USERLAND_BUILD)/dsh/%.dobj: %.c
	mkdir -p $(dir $@)
	$(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -c -o $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/dsh/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -c -o $@ $<

$(DSH_DXR): $(DSH_C_OBJS) $(DSH_S_OBJS)
	libgcc=`$(PDP10_GCC) -print-libgcc-file-name`; \
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(DSH_MAP) $(DSH_S_OBJS) $(DSH_C_OBJS) $$libgcc

clean:
	rm -rf $(USERLAND_BUILD)
