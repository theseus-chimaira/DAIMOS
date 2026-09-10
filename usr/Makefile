PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
USR_BUILD := $(BUILD_ROOT)/usr
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
DAS ?= $(PDP10_PREFIX)/bin/das
DLINK ?= $(PDP10_PREFIX)/bin/dlink
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel/boot -I$(PROJECT_ROOT)/system/kernel/core -I$(PROJECT_ROOT)/system/kernel/drivers -I$(PROJECT_ROOT)/system/kernel/fs -I$(PROJECT_ROOT)/system/kernel/mm -I$(PROJECT_ROOT)/system/kernel/modules -I$(PROJECT_ROOT)/system/kernel/proc -I$(PROJECT_ROOT)/system/kernel/storage -I$(PROJECT_ROOT)/usr/libc -I$(PROJECT_ROOT)/usr/exec
CFLAGS = -std=c99 -Os

CHECK_SRCS = libc/u.c exec/commands.c exec/standalone.c dsh/dsh.c
CHECK_ASMS = $(CHECK_SRCS:%.c=$(USR_BUILD)/check/%.s)
DSH_C_SRCS = libc/u.c exec/commands.c dsh/dsh.c
DSH_C_OBJS = $(DSH_C_SRCS:%.c=$(USR_BUILD)/dsh/%.dobj)
DSH_S_SRCS = libc/crt0.s libc/syscall_helpers.s
DSH_S_OBJS = $(DSH_S_SRCS:%.s=$(USR_BUILD)/dsh/%.dobj)
DSH_DXR = $(USR_BUILD)/dsh.dxr
DSH_MAP = $(USR_BUILD)/dsh.map

.DEFAULT_GOAL := build
.PHONY: build check clean

build: $(DSH_DXR)

check: $(CHECK_ASMS) $(DSH_DXR)

$(USR_BUILD)/check/%.s: %.c
	mkdir -p $(dir $@)
	$(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -S $< -o $@

$(USR_BUILD)/dsh/%.dobj: %.c
	mkdir -p $(dir $@)
	$(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) -c -o $@ $(@:.dobj=.s)

$(USR_BUILD)/dsh/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) -c -o $@ $<

$(DSH_DXR): $(DSH_C_OBJS) $(DSH_S_OBJS)
	libgcc=`$(PDP10_GCC) -print-libgcc-file-name`; \
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(DSH_MAP) $(DSH_S_OBJS) $(DSH_C_OBJS) $$libgcc

clean:
	rm -rf $(USR_BUILD)
