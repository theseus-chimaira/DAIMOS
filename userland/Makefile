PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
USERLAND_BUILD := $(BUILD_ROOT)/userland
KCC ?= $(PDP10_PREFIX)/bin/kcc
DAS ?= $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -F
DLINK ?= $(PDP10_PREFIX)/bin/dlink
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel/boot -I$(PROJECT_ROOT)/system/kernel/core -I$(PROJECT_ROOT)/system/kernel/drivers -I$(PROJECT_ROOT)/system/kernel/fs -I$(PROJECT_ROOT)/system/kernel/mm -I$(PROJECT_ROOT)/system/kernel/modules -I$(PROJECT_ROOT)/system/kernel/proc -I$(PROJECT_ROOT)/system/kernel/storage -I$(PROJECT_ROOT)/userland/libc -I$(PROJECT_ROOT)/userland/exec -I$(PDP10_PREFIX)/include
KCCFLAGS ?= -Pgnu99 -O -x=pdp6 -m=gas

CHECK_SRCS = libc/u.c libc/text.c exec/commands.c exec/standalone.c dsh/dsh.c dsh/dsh_s6.c dsh/dsh_lex.c dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c init/init.c login/login.c tsfs/tsfs_scan.c tsfs/tsfsprobe.c tsfs/mount_tsfs.c dtfs/dtfs_probe.c dtfs/dtfsprobe.c d6lz/d6lz.c logstore/logdrain.c
CHECK_ASMS = $(CHECK_SRCS:%.c=$(USERLAND_BUILD)/check/%.s)
COMMON_C_SRCS = libc/u.c libc/text.c
COMMON_S_SRCS = libc/crt0.s libc/syscall.s libc/syscall_helpers.s

INIT_C_SRCS = $(COMMON_C_SRCS) libc/logevent.c init/init.c
INIT_C_OBJS = $(INIT_C_SRCS:%.c=$(USERLAND_BUILD)/init/%.dobj)
INIT_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/init/%.dobj)
INIT_DXR = $(USERLAND_BUILD)/init.dxr
INIT_MAP = $(USERLAND_BUILD)/init.map

MOUNTALL_C_SRCS = $(COMMON_C_SRCS) init/mountall.c
MOUNTALL_C_OBJS = $(MOUNTALL_C_SRCS:%.c=$(USERLAND_BUILD)/mountall/%.dobj)
MOUNTALL_S_SRCS = $(COMMON_S_SRCS) init/memfs_mount.s
MOUNTALL_S_OBJS = $(MOUNTALL_S_SRCS:%.s=$(USERLAND_BUILD)/mountall/%.dobj)
MOUNTALL_DXR = $(USERLAND_BUILD)/mountall.dxr
MOUNTALL_MAP = $(USERLAND_BUILD)/mountall.map

LOGIN_C_SRCS = $(COMMON_C_SRCS) libc/logevent.c login/login.c
LOGIN_C_OBJS = $(LOGIN_C_SRCS:%.c=$(USERLAND_BUILD)/login/%.dobj)
LOGIN_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/login/%.dobj)
LOGIN_DXR = $(USERLAND_BUILD)/login.dxr
LOGIN_MAP = $(USERLAND_BUILD)/login.map

DSH_C_SRCS = libc/u.c libc/text.c dsh/dsh_s6.c dsh/dsh_lex.c dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c dsh/dsh.c
DSH_C_OBJS = $(DSH_C_SRCS:%.c=$(USERLAND_BUILD)/dsh/%.dobj)
DSH_S_SRCS = $(COMMON_S_SRCS)
DSH_S_OBJS = $(DSH_S_SRCS:%.s=$(USERLAND_BUILD)/dsh/%.dobj)
DSH_DXR = $(USERLAND_BUILD)/dsh.dxr
DSH_MAP = $(USERLAND_BUILD)/dsh.map

COMMAND_C_SRCS = $(COMMON_C_SRCS) exec/commands.c exec/standalone.c
COMMAND_C_OBJS = $(COMMAND_C_SRCS:%.c=$(USERLAND_BUILD)/command/%.dobj)
COMMAND_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/command/%.dobj)
COMMAND_DXR = $(USERLAND_BUILD)/command.dxr
COMMAND_MAP = $(USERLAND_BUILD)/command.map

TSFSPROBE_C_SRCS = $(COMMON_C_SRCS) tsfs/tsfs_scan.c tsfs/tsfsprobe.c
TSFSPROBE_C_OBJS = $(TSFSPROBE_C_SRCS:%.c=$(USERLAND_BUILD)/tsfsprobe/%.dobj)
TSFSPROBE_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/tsfsprobe/%.dobj)
TSFSPROBE_DXR = $(USERLAND_BUILD)/tsfsprobe.dxr
TSFSPROBE_MAP = $(USERLAND_BUILD)/tsfsprobe.map

MOUNTTSFS_C_SRCS = $(COMMON_C_SRCS) tsfs/tsfs_scan.c tsfs/mount_tsfs.c
MOUNTTSFS_C_OBJS = $(MOUNTTSFS_C_SRCS:%.c=$(USERLAND_BUILD)/mounttsfs/%.dobj)
MOUNTTSFS_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/mounttsfs/%.dobj)
MOUNTTSFS_DXR = $(USERLAND_BUILD)/mounttsfs.dxr
MOUNTTSFS_MAP = $(USERLAND_BUILD)/mounttsfs.map

DTFSPROBE_C_SRCS = $(COMMON_C_SRCS) dtfs/dtfs_probe.c dtfs/dtfsprobe.c
DTFSPROBE_C_OBJS = $(DTFSPROBE_C_SRCS:%.c=$(USERLAND_BUILD)/dtfsprobe/%.dobj)
DTFSPROBE_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/dtfsprobe/%.dobj)
DTFSPROBE_DXR = $(USERLAND_BUILD)/dtfsprobe.dxr
DTFSPROBE_MAP = $(USERLAND_BUILD)/dtfsprobe.map

D6LZ_C_SRCS = libc/u.c d6lz/d6lz.c
D6LZ_C_OBJS = $(D6LZ_C_SRCS:%.c=$(USERLAND_BUILD)/d6lz/%.dobj)
D6LZ_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/d6lz/%.dobj)
D6LZ_DXR = $(USERLAND_BUILD)/d6lz.dxr
D6LZ_MAP = $(USERLAND_BUILD)/d6lz.map

LOGDRAIN_C_SRCS = $(COMMON_C_SRCS) logstore/logdrain.c
LOGDRAIN_C_OBJS = $(LOGDRAIN_C_SRCS:%.c=$(USERLAND_BUILD)/logdrain/%.dobj)
LOGDRAIN_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/logdrain/%.dobj)
LOGDRAIN_DXR = $(USERLAND_BUILD)/logdrain.dxr
LOGDRAIN_MAP = $(USERLAND_BUILD)/logdrain.map

.DEFAULT_GOAL := build
.PHONY: build check clean

build: $(INIT_DXR) $(MOUNTALL_DXR) $(LOGIN_DXR) $(DSH_DXR) $(COMMAND_DXR) $(TSFSPROBE_DXR) $(MOUNTTSFS_DXR) $(DTFSPROBE_DXR) $(D6LZ_DXR) $(LOGDRAIN_DXR)

check: $(CHECK_ASMS) build

$(USERLAND_BUILD)/check/%.s: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $@

$(USERLAND_BUILD)/init/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/init/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(USERLAND_BUILD)/login/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/login/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(USERLAND_BUILD)/dsh/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/dsh/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(INIT_DXR): $(INIT_C_OBJS) $(INIT_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(INIT_MAP) $(INIT_S_OBJS) $(INIT_C_OBJS)

$(USERLAND_BUILD)/mountall/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/mountall/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(MOUNTALL_DXR): $(MOUNTALL_C_OBJS) $(MOUNTALL_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(MOUNTALL_MAP) $(MOUNTALL_S_OBJS) $(MOUNTALL_C_OBJS)

$(LOGIN_DXR): $(LOGIN_C_OBJS) $(LOGIN_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(LOGIN_MAP) $(LOGIN_S_OBJS) $(LOGIN_C_OBJS)

$(DSH_DXR): $(DSH_C_OBJS) $(DSH_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(DSH_MAP) $(DSH_S_OBJS) $(DSH_C_OBJS)

$(USERLAND_BUILD)/command/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/command/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(COMMAND_DXR): $(COMMAND_C_OBJS) $(COMMAND_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(COMMAND_MAP) $(COMMAND_S_OBJS) $(COMMAND_C_OBJS)

$(USERLAND_BUILD)/tsfsprobe/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/tsfsprobe/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(TSFSPROBE_DXR): $(TSFSPROBE_C_OBJS) $(TSFSPROBE_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(TSFSPROBE_MAP) $(TSFSPROBE_S_OBJS) $(TSFSPROBE_C_OBJS)

$(USERLAND_BUILD)/mounttsfs/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/mounttsfs/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(MOUNTTSFS_DXR): $(MOUNTTSFS_C_OBJS) $(MOUNTTSFS_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(MOUNTTSFS_MAP) $(MOUNTTSFS_S_OBJS) $(MOUNTTSFS_C_OBJS)

$(USERLAND_BUILD)/dtfsprobe/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/dtfsprobe/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(DTFSPROBE_DXR): $(DTFSPROBE_C_OBJS) $(DTFSPROBE_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(DTFSPROBE_MAP) $(DTFSPROBE_S_OBJS) $(DTFSPROBE_C_OBJS)

$(USERLAND_BUILD)/d6lz/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/d6lz/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(D6LZ_DXR): $(D6LZ_C_OBJS) $(D6LZ_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(D6LZ_MAP) $(D6LZ_S_OBJS) $(D6LZ_C_OBJS)

$(USERLAND_BUILD)/logdrain/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/logdrain/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(LOGDRAIN_DXR): $(LOGDRAIN_C_OBJS) $(LOGDRAIN_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(LOGDRAIN_MAP) $(LOGDRAIN_S_OBJS) $(LOGDRAIN_C_OBJS)

clean:
	rm -rf $(USERLAND_BUILD)
