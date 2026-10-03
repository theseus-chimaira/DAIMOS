PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
USERLAND_BUILD := $(BUILD_ROOT)/userland
KCC ?= $(PDP10_PREFIX)/bin/kcc
DAS ?= $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -F
DLINK ?= $(PDP10_PREFIX)/bin/dlink
P10FOLD ?= $(PDP10_PREFIX)/bin/p10fold
SIXMD_CHECK ?= $(PDP10_PREFIX)/bin/sixmd-check
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel/boot -I$(PROJECT_ROOT)/system/kernel/core -I$(PROJECT_ROOT)/system/kernel/drivers -I$(PROJECT_ROOT)/system/kernel/fs -I$(PROJECT_ROOT)/system/kernel/mm -I$(PROJECT_ROOT)/system/kernel/modules -I$(PROJECT_ROOT)/system/kernel/proc -I$(PROJECT_ROOT)/system/kernel/storage -I$(PROJECT_ROOT)/userland/libc -I$(PROJECT_ROOT)/userland/exec -I$(PDP10_PREFIX)/include
KCCFLAGS ?= -Pgnu99 -O -x=pdp6 -m=gas
KCC_HEADERS = $(wildcard $(PROJECT_ROOT)/userland/libc/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/exec/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/dsh/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/init/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/login/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/tsfs/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/dtfs/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/d6lz/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/logstore/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/boot/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/core/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/drivers/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/fs/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/mm/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/modules/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/proc/*.h) \
	$(wildcard $(PROJECT_ROOT)/system/kernel/storage/*.h) \
	$(wildcard $(PDP10_PREFIX)/include/*.h)

CHECK_SRCS = libc/u.c libc/text.c exec/commands.c exec/standalone.c exec/utility.c exec/utility_standalone.c exec/xargs.c exec/sed.c exec/configctl.c exec/make.c dsh/dsh.c dsh/dsh_s6.c dsh/dsh_lex.c dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c init/init.c login/login.c tsfs/tsfs_scan.c tsfs/tsfsprobe.c tsfs/mount_tsfs.c dtfs/dtfs_probe.c dtfs/dtfsprobe.c d6lz/d6lz.c logstore/logdrain.c logstore/logcompat.c
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

DSH_C_SRCS = libc/u.c libc/text.c libc/cmdmap.c dsh/dsh_s6.c dsh/dsh_lex.c dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c dsh/dsh.c
DSH_C_OBJS = $(DSH_C_SRCS:%.c=$(USERLAND_BUILD)/dsh/%.dobj)
DSH_S_SRCS = $(COMMON_S_SRCS) dsh/dsh_var_pdp6.s
DSH_S_OBJS = $(DSH_S_SRCS:%.s=$(USERLAND_BUILD)/dsh/%.dobj)
DSH_DXR = $(USERLAND_BUILD)/dsh.dxr
DSH_MAP = $(USERLAND_BUILD)/dsh.map
DSH_FOLD_PLAN = $(USERLAND_BUILD)/dsh.fold

DSHCOMP_C_SRCS = $(COMMON_C_SRCS) dsh/dshcomp.c
DSHCOMP_C_OBJS = $(DSHCOMP_C_SRCS:%.c=$(USERLAND_BUILD)/dshcomp/%.dobj)
DSHCOMP_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/dshcomp/%.dobj)
DSHCOMP_DXR = $(USERLAND_BUILD)/dshcomp.dxr
DSHCOMP_MAP = $(USERLAND_BUILD)/dshcomp.map

COMMAND_C_SRCS = $(COMMON_C_SRCS) exec/commands.c exec/standalone.c
COMMAND_C_OBJS = $(COMMAND_C_SRCS:%.c=$(USERLAND_BUILD)/command/%.dobj)
COMMAND_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/command/%.dobj)
COMMAND_DXR = $(USERLAND_BUILD)/command.dxr
COMMAND_MAP = $(USERLAND_BUILD)/command.map

UTILITY_C_SRCS = $(COMMON_C_SRCS) exec/utility.c exec/utility_standalone.c
UTILITY_MISC_C_OBJS = $(UTILITY_C_SRCS:%.c=$(USERLAND_BUILD)/utility-misc/%.dobj)
UTILITY_MISC_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-misc/%.dobj)
UTILITY_MISC_DXR = $(USERLAND_BUILD)/utility-misc.dxr
UTILITY_MISC_MAP = $(USERLAND_BUILD)/utility-misc.map
UTILITY_TEXT_C_OBJS = $(UTILITY_C_SRCS:%.c=$(USERLAND_BUILD)/utility-text/%.dobj)
UTILITY_TEXT_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-text/%.dobj)
UTILITY_TEXT_DXR = $(USERLAND_BUILD)/utility-text.dxr
UTILITY_TEXT_MAP = $(USERLAND_BUILD)/utility-text.map
UTILITY_DOC_C_OBJS = $(UTILITY_C_SRCS:%.c=$(USERLAND_BUILD)/utility-doc/%.dobj)
UTILITY_DOC_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-doc/%.dobj)
UTILITY_DOC_DXR = $(USERLAND_BUILD)/utility-doc.dxr
UTILITY_DOC_MAP = $(USERLAND_BUILD)/utility-doc.map

XARGS_C_SRCS = exec/xargs.c
XARGS_C_OBJS = $(XARGS_C_SRCS:%.c=$(USERLAND_BUILD)/xargs/%.dobj)
XARGS_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/xargs/%.dobj)
XARGS_DXR = $(USERLAND_BUILD)/xargs.dxr
XARGS_MAP = $(USERLAND_BUILD)/xargs.map

SED_C_SRCS = $(COMMON_C_SRCS) exec/sed.c
SED_C_OBJS = $(SED_C_SRCS:%.c=$(USERLAND_BUILD)/sed/%.dobj)
SED_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/sed/%.dobj)
SED_DXR = $(USERLAND_BUILD)/sed.dxr
SED_MAP = $(USERLAND_BUILD)/sed.map

CONFIGCTL_C_SRCS = $(COMMON_C_SRCS) exec/configctl.c
CONFIGCTL_C_OBJS = $(CONFIGCTL_C_SRCS:%.c=$(USERLAND_BUILD)/configctl/%.dobj)
CONFIGCTL_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/configctl/%.dobj)
CONFIGCTL_DXR = $(USERLAND_BUILD)/configctl.dxr
CONFIGCTL_MAP = $(USERLAND_BUILD)/configctl.map

MAKE_C_SRCS = $(COMMON_C_SRCS) exec/make.c
MAKE_C_OBJS = $(MAKE_C_SRCS:%.c=$(USERLAND_BUILD)/make/%.dobj)
MAKE_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/make/%.dobj)
MAKE_DXR = $(USERLAND_BUILD)/make.dxr
MAKE_MAP = $(USERLAND_BUILD)/make.map

LOGCOMPAT_C_SRCS = $(COMMON_C_SRCS) libc/logevent.c logstore/logcompat.c
LOGCOMPAT_C_OBJS = $(LOGCOMPAT_C_SRCS:%.c=$(USERLAND_BUILD)/logcompat/%.dobj)
LOGCOMPAT_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/logcompat/%.dobj)
LOGCOMPAT_DXR = $(USERLAND_BUILD)/logcompat.dxr
LOGCOMPAT_MAP = $(USERLAND_BUILD)/logcompat.map

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

KCC_GENERATED = $(CHECK_ASMS) $(INIT_C_OBJS) $(MOUNTALL_C_OBJS) \
	$(LOGIN_C_OBJS) $(DSH_C_OBJS) $(COMMAND_C_OBJS) \
	$(DSHCOMP_C_OBJS) \
	$(UTILITY_MISC_C_OBJS) $(UTILITY_TEXT_C_OBJS) $(UTILITY_DOC_C_OBJS) \
	$(XARGS_C_OBJS) $(SED_C_OBJS) $(CONFIGCTL_C_OBJS) $(MAKE_C_OBJS) \
	$(LOGCOMPAT_C_OBJS) $(TSFSPROBE_C_OBJS) $(MOUNTTSFS_C_OBJS) \
	$(DTFSPROBE_C_OBJS) $(D6LZ_C_OBJS) $(LOGDRAIN_C_OBJS)

$(KCC_GENERATED): $(KCC) $(KCC_HEADERS)

.DEFAULT_GOAL := build
.PHONY: build check manual-check clean

build: manual-check $(INIT_DXR) $(MOUNTALL_DXR) $(LOGIN_DXR) $(DSH_DXR) $(DSHCOMP_DXR) $(COMMAND_DXR) $(UTILITY_MISC_DXR) $(UTILITY_TEXT_DXR) $(UTILITY_DOC_DXR) $(XARGS_DXR) $(SED_DXR) $(CONFIGCTL_DXR) $(MAKE_DXR) $(LOGCOMPAT_DXR) $(TSFSPROBE_DXR) $(MOUNTTSFS_DXR) $(DTFSPROBE_DXR) $(D6LZ_DXR) $(LOGDRAIN_DXR)

PUBLIC_MANUAL_NAMES = $(shell awk '{ n = $$1; if (n == "[") n = "BRACKET"; print n; }' exec/MAP) \
	INIT LOGIN DSH XARGS SED CONFIGCTL MAKE TSFSPROBE MOUNT.TSFS DTFSPROBE \
	D6LZ LOGDRAIN
PUBLIC_MANUAL_FILES = $(addprefix manual/,$(addsuffix .SIXMD,$(PUBLIC_MANUAL_NAMES)))

manual-check: $(PUBLIC_MANUAL_FILES)
	@set -eu; for f in $(PUBLIC_MANUAL_FILES); do \
		n=$$(basename "$$f" .SIXMD); \
		$(SIXMD_CHECK) "$$f" "$$n"; \
	done

check: $(CHECK_ASMS) build manual-check

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

$(USERLAND_BUILD)/dshcomp/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/dshcomp/%.dobj: %.s
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

$(DSH_FOLD_PLAN): $(DSH_S_OBJS) $(P10FOLD) Makefile
	$(P10FOLD) -m 4 -P $@ $(DSH_S_OBJS) >/dev/null

$(DSH_DXR): $(DSH_C_OBJS) $(DSH_S_OBJS) $(DSH_FOLD_PLAN)
	$(DLINK) --daimos-uuo-relax -b 020 --fold-plan $(DSH_FOLD_PLAN) -o $@ -M $(DSH_MAP) $(DSH_S_OBJS) $(DSH_C_OBJS)

$(DSHCOMP_DXR): $(DSHCOMP_C_OBJS) $(DSHCOMP_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(DSHCOMP_MAP) $(DSHCOMP_S_OBJS) $(DSHCOMP_C_OBJS)

$(USERLAND_BUILD)/command/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/command/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(COMMAND_DXR): $(COMMAND_C_OBJS) $(COMMAND_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(COMMAND_MAP) $(COMMAND_S_OBJS) $(COMMAND_C_OBJS)

$(USERLAND_BUILD)/utility-misc/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -DUTIL_GROUP=1 -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/utility-misc/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(UTILITY_MISC_DXR): $(UTILITY_MISC_C_OBJS) $(UTILITY_MISC_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(UTILITY_MISC_MAP) $(UTILITY_MISC_S_OBJS) $(UTILITY_MISC_C_OBJS)

$(USERLAND_BUILD)/utility-text/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -DUTIL_GROUP=2 -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/utility-text/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(UTILITY_TEXT_DXR): $(UTILITY_TEXT_C_OBJS) $(UTILITY_TEXT_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(UTILITY_TEXT_MAP) $(UTILITY_TEXT_S_OBJS) $(UTILITY_TEXT_C_OBJS)

$(USERLAND_BUILD)/utility-doc/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -DUTIL_GROUP=3 -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/utility-doc/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(UTILITY_DOC_DXR): $(UTILITY_DOC_C_OBJS) $(UTILITY_DOC_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(UTILITY_DOC_MAP) $(UTILITY_DOC_S_OBJS) $(UTILITY_DOC_C_OBJS)

$(USERLAND_BUILD)/xargs/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/xargs/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(XARGS_DXR): $(XARGS_C_OBJS) $(XARGS_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(XARGS_MAP) $(XARGS_S_OBJS) $(XARGS_C_OBJS)

$(USERLAND_BUILD)/sed/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/sed/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(SED_DXR): $(SED_C_OBJS) $(SED_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(SED_MAP) $(SED_S_OBJS) $(SED_C_OBJS)

$(USERLAND_BUILD)/configctl/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/configctl/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(CONFIGCTL_DXR): $(CONFIGCTL_C_OBJS) $(CONFIGCTL_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(CONFIGCTL_MAP) $(CONFIGCTL_S_OBJS) $(CONFIGCTL_C_OBJS)

$(USERLAND_BUILD)/make/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/make/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(MAKE_DXR): $(MAKE_C_OBJS) $(MAKE_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(MAKE_MAP) $(MAKE_S_OBJS) $(MAKE_C_OBJS)

$(USERLAND_BUILD)/logcompat/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/logcompat/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(LOGCOMPAT_DXR): $(LOGCOMPAT_C_OBJS) $(LOGCOMPAT_S_OBJS)
	$(DLINK) --daimos-uuo-relax -b 020 -o $@ -M $(LOGCOMPAT_MAP) $(LOGCOMPAT_S_OBJS) $(LOGCOMPAT_C_OBJS)

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
