PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
USER_BUILD := $(BUILD_ROOT)/userland-v1
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
DAS ?= $(PDP10_PREFIX)/bin/das
DLINK ?= $(PDP10_PREFIX)/bin/dlink
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel -I$(PROJECT_ROOT)/userland/v1 -I$(PROJECT_ROOT)/userland/exec_v1
CFLAGS = -std=c99 -Os

CHECK_SRCS = v1/u_v1.c exec_v1/commands_v1.c exec_v1/standalone.c dsh_v1/dsh.c
CHECK_ASMS = $(CHECK_SRCS:%.c=$(USER_BUILD)/check/%.s)
DSH_C_SRCS = v1/u_v1.c exec_v1/commands_v1.c dsh_v1/dsh.c
DSH_C_OBJS = $(DSH_C_SRCS:%.c=$(USER_BUILD)/dsh/%.dobj)
DSH_S_SRCS = v1/crt0_v1.s v1/syscall_v1.s v1/syscall_sentinel_v1.s
DSH_S_OBJS = $(DSH_S_SRCS:%.s=$(USER_BUILD)/dsh/%.dobj)
DSH_DXR = $(USER_BUILD)/dsh-v1.dxr
DSH_MAP = $(USER_BUILD)/dsh-v1.map

.DEFAULT_GOAL := build
.PHONY: build check clean

build: $(DSH_DXR)

check: $(CHECK_ASMS) $(DSH_DXR)

$(USER_BUILD)/check/%.s: %.c
	mkdir -p $(dir $@)
	$(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -S $< -o $@

$(USER_BUILD)/dsh/%.dobj: %.c
	mkdir -p $(dir $@)
	PATH='$(PDP10_PREFIX)/bin:$(PATH)' $(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(USER_BUILD)/dsh/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) -c -o $@ $<

$(DSH_DXR): $(DSH_C_OBJS) $(DSH_S_OBJS)
	libgcc=`$(PDP10_GCC) -print-libgcc-file-name`; \
	$(DLINK) -o $@ -M $(DSH_MAP) $(DSH_S_OBJS) $(DSH_C_OBJS) $$libgcc

clean:
	rm -rf $(USER_BUILD)
