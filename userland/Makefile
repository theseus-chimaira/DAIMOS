PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
USERLAND_BUILD := $(BUILD_ROOT)/userland
KCC ?= $(PDP10_PREFIX)/bin/kcc
DAS ?= $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -F
DLINK ?= $(PDP10_PREFIX)/bin/dlink
DARC ?= $(PDP10_PREFIX)/bin/darc
P10FOLD ?= $(PDP10_PREFIX)/bin/p10fold
SIXMD_CHECK ?= $(PDP10_PREFIX)/bin/sixmd-check
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel/boot -I$(PROJECT_ROOT)/system/kernel/core -I$(PROJECT_ROOT)/system/kernel/drivers -I$(PROJECT_ROOT)/system/kernel/fs -I$(PROJECT_ROOT)/system/kernel/mm -I$(PROJECT_ROOT)/system/kernel/modules -I$(PROJECT_ROOT)/system/kernel/proc -I$(PROJECT_ROOT)/system/kernel/storage -I$(PROJECT_ROOT)/userland/libc -I$(PROJECT_ROOT)/userland/exec -I$(PDP10_PREFIX)/include
KCCFLAGS ?= -Pgnu99 -O -x=pdp6 -m=gas
KCC_HEADERS = $(wildcard $(PROJECT_ROOT)/userland/libc/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/exec/*.h) \
	$(wildcard $(PROJECT_ROOT)/userland/toolchain/*.h) \
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

CHECK_SRCS = libc/u.c libc/text.c exec/commands.c exec/standalone.c exec/utility.c exec/utility_standalone.c exec/xargs.c exec/sed.c exec/configctl.c exec/make.c dsh/dsh.c dsh/dsh_s6.c dsh/dsh_lex.c dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c init/init.c login/login.c tsfs/tsfs_scan.c tsfs/tsfsprobe.c tsfs/mount_tsfs.c dtfs/dtfs_probe.c dtfs/dtfsprobe.c d6lz/d6lz.c logstore/logdrain.c logstore/logcompat.c toolchain/dobj.c toolchain/darc.c toolchain/dlink.c toolchain/objdump.c
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

COMMAND_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/command/%.dobj)

UTILITY_C_SRCS = $(COMMON_C_SRCS) exec/utility.c exec/utility_standalone.c
UTILITY_MISC_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-misc/%.dobj)
UTILITY_TEXT_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-text/%.dobj)
UTILITY_DOC_S_OBJS = $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-doc/%.dobj)

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

LOGD_C_SRCS = $(COMMON_C_SRCS) libc/logevent.c logstore/logcompat.c
LOGDUMP_C_SRCS = $(COMMON_C_SRCS) logstore/logcompat.c
LOGD_DXR = $(USERLAND_BUILD)/program/LOGD.dxr
LOGDUMP_DXR = $(USERLAND_BUILD)/program/LOGDUMP.dxr

# Public commands are ordinary executables.  Their implementation objects are
# kept in static archives so shared implementation code is a link-time detail,
# not a runtime command-map ABI.  The archives can later become shared
# libraries without changing /SYSTEM/EXEC or command lookup semantics.
COMMAND_PROGRAMS = ECHO CAT LS MKDIR RM PWD STAT TOUCH DATE RMDIR CP CHMOD INSTALL \
	CHOWN MKFS.DTFS FSCK.DTFS MOUNT MOUNT.DTFS UNMOUNT MV HEXDUMP PS DEVS \
	MODS MOUNTS FREE MEMSTAT SYSCTL DF TTYOUT HALT CLEAR DPYVIEW KILL
UTILITY_MISC_PROGRAMS = TRUE FALSE CHECK BASENAME DIRNAME ENV WHICH EXPR BC \
	TEST BRACKET CAL
UTILITY_TEXT_PROGRAMS = CMP HEAD GREP WC TEE TAIL CUT SORT DD STRINGS
UTILITY_DOC_PROGRAMS = MAN HELP APROPOS TREE FIND DU

COMMAND_LIB = $(USERLAND_BUILD)/libcommand.darc
UTILITY_MISC_LIB = $(USERLAND_BUILD)/libutility-misc.darc
UTILITY_TEXT_LIB = $(USERLAND_BUILD)/libutility-text.darc
UTILITY_DOC_LIB = $(USERLAND_BUILD)/libutility-doc.darc
COMMAND_MEMBER_OBJS = $(addprefix $(USERLAND_BUILD)/command-member/,$(addsuffix .dobj,$(COMMAND_PROGRAMS)))
COMMAND_COMMON_OBJS = $(COMMON_C_SRCS:%.c=$(USERLAND_BUILD)/command/%.dobj)
COMMAND_LIB_OBJS = $(COMMAND_MEMBER_OBJS) $(COMMAND_COMMON_OBJS)
UTILITY_MISC_MEMBER_OBJS = $(addprefix $(USERLAND_BUILD)/utility-member-misc/,$(addsuffix .dobj,$(UTILITY_MISC_PROGRAMS)))
UTILITY_TEXT_MEMBER_OBJS = $(addprefix $(USERLAND_BUILD)/utility-member-text/,$(addsuffix .dobj,$(UTILITY_TEXT_PROGRAMS)))
UTILITY_DOC_MEMBER_OBJS = $(addprefix $(USERLAND_BUILD)/utility-member-doc/,$(addsuffix .dobj,$(UTILITY_DOC_PROGRAMS)))
UTILITY_MISC_COMMON_OBJS = $(COMMON_C_SRCS:%.c=$(USERLAND_BUILD)/utility-misc/%.dobj)
UTILITY_TEXT_COMMON_OBJS = $(COMMON_C_SRCS:%.c=$(USERLAND_BUILD)/utility-text/%.dobj)
UTILITY_DOC_COMMON_OBJS = $(COMMON_C_SRCS:%.c=$(USERLAND_BUILD)/utility-doc/%.dobj)
UTILITY_MISC_LIB_OBJS = $(UTILITY_MISC_MEMBER_OBJS) $(UTILITY_MISC_COMMON_OBJS)
UTILITY_TEXT_LIB_OBJS = $(UTILITY_TEXT_MEMBER_OBJS) $(UTILITY_TEXT_COMMON_OBJS)
UTILITY_DOC_LIB_OBJS = $(UTILITY_DOC_MEMBER_OBJS) $(UTILITY_DOC_COMMON_OBJS)

COMMAND_PROGRAM_DXRS = $(addprefix $(USERLAND_BUILD)/program/,$(addsuffix .dxr,$(COMMAND_PROGRAMS)))
UTILITY_MISC_PROGRAM_DXRS = $(addprefix $(USERLAND_BUILD)/program/,$(addsuffix .dxr,$(UTILITY_MISC_PROGRAMS)))
UTILITY_TEXT_PROGRAM_DXRS = $(addprefix $(USERLAND_BUILD)/program/,$(addsuffix .dxr,$(UTILITY_TEXT_PROGRAMS)))
UTILITY_DOC_PROGRAM_DXRS = $(addprefix $(USERLAND_BUILD)/program/,$(addsuffix .dxr,$(UTILITY_DOC_PROGRAMS)))
PUBLIC_PROGRAM_DXRS = $(COMMAND_PROGRAM_DXRS) $(UTILITY_MISC_PROGRAM_DXRS) \
	$(UTILITY_TEXT_PROGRAM_DXRS) $(UTILITY_DOC_PROGRAM_DXRS)

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

TOOLCHAIN_COMMON_C_SRCS = libc/u.c toolchain/dobj.c
TOOLCHAIN_S_SRCS = $(COMMON_S_SRCS)

DARC_C_SRCS = $(TOOLCHAIN_COMMON_C_SRCS) toolchain/darc.c
DARC_C_OBJS = $(DARC_C_SRCS:%.c=$(USERLAND_BUILD)/darc/%.dobj)
DARC_S_OBJS = $(TOOLCHAIN_S_SRCS:%.s=$(USERLAND_BUILD)/darc/%.dobj)
DARC_DXR = $(USERLAND_BUILD)/darc.dxr
DARC_MAP = $(USERLAND_BUILD)/darc.map

DLINK_C_SRCS = $(TOOLCHAIN_COMMON_C_SRCS) toolchain/dlink.c
DLINK_C_OBJS = $(DLINK_C_SRCS:%.c=$(USERLAND_BUILD)/dlink/%.dobj)
DLINK_S_OBJS = $(TOOLCHAIN_S_SRCS:%.s=$(USERLAND_BUILD)/dlink/%.dobj)
DLINK_DXR = $(USERLAND_BUILD)/dlink.dxr
DLINK_MAP = $(USERLAND_BUILD)/dlink.map

OBJDUMP_C_SRCS = $(TOOLCHAIN_COMMON_C_SRCS) toolchain/objdump.c
OBJDUMP_C_OBJS = $(OBJDUMP_C_SRCS:%.c=$(USERLAND_BUILD)/objdump/%.dobj)
OBJDUMP_S_OBJS = $(TOOLCHAIN_S_SRCS:%.s=$(USERLAND_BUILD)/objdump/%.dobj)
OBJDUMP_DXR = $(USERLAND_BUILD)/objdump.dxr
OBJDUMP_MAP = $(USERLAND_BUILD)/objdump.map

KCC_GENERATED = $(CHECK_ASMS) $(INIT_C_OBJS) $(MOUNTALL_C_OBJS) \
	$(LOGIN_C_OBJS) $(DSH_C_OBJS) \
	$(DSHCOMP_C_OBJS) \
	$(XARGS_C_OBJS) $(SED_C_OBJS) $(CONFIGCTL_C_OBJS) $(MAKE_C_OBJS) \
	$(TSFSPROBE_C_OBJS) $(MOUNTTSFS_C_OBJS) \
	$(DTFSPROBE_C_OBJS) $(D6LZ_C_OBJS) $(LOGDRAIN_C_OBJS) \
	$(DARC_C_OBJS) $(DLINK_C_OBJS) $(OBJDUMP_C_OBJS)

$(KCC_GENERATED): $(KCC) $(KCC_HEADERS)

.DEFAULT_GOAL := build
.PHONY: build check manual-check clean

build: manual-check $(INIT_DXR) $(MOUNTALL_DXR) $(LOGIN_DXR) $(DSH_DXR) $(DSHCOMP_DXR) $(PUBLIC_PROGRAM_DXRS) $(XARGS_DXR) $(SED_DXR) $(CONFIGCTL_DXR) $(MAKE_DXR) $(LOGD_DXR) $(LOGDUMP_DXR) $(TSFSPROBE_DXR) $(MOUNTTSFS_DXR) $(DTFSPROBE_DXR) $(D6LZ_DXR) $(LOGDRAIN_DXR) $(DARC_DXR) $(DLINK_DXR) $(OBJDUMP_DXR)

PUBLIC_MANUAL_NAMES = $(COMMAND_PROGRAMS) $(UTILITY_MISC_PROGRAMS) \
	$(UTILITY_TEXT_PROGRAMS) $(UTILITY_DOC_PROGRAMS) LOGD LOGDUMP \
	INIT LOGIN DSH XARGS SED CONFIGCTL MAKE TSFSPROBE MOUNT.TSFS DTFSPROBE \
	D6LZ LOGDRAIN DARC DLINK OBJDUMP
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

$(COMMAND_LIB): $(COMMAND_LIB_OBJS)
	$(DARC) -o $@ $(COMMAND_LIB_OBJS)

$(UTILITY_MISC_LIB): $(UTILITY_MISC_LIB_OBJS)
	$(DARC) -o $@ $(UTILITY_MISC_LIB_OBJS)

$(UTILITY_TEXT_LIB): $(UTILITY_TEXT_LIB_OBJS)
	$(DARC) -o $@ $(UTILITY_TEXT_LIB_OBJS)

$(UTILITY_DOC_LIB): $(UTILITY_DOC_LIB_OBJS)
	$(DARC) -o $@ $(UTILITY_DOC_LIB_OBJS)

define COMMAND_PROGRAM_template
$(USERLAND_BUILD)/command-member/$(1).dobj: exec/commands.c exec/cmd.h
	mkdir -p $$(dir $$@)
	$$(KCC) $$(KCCFLAGS) $$(CPPFLAGS) \
	    -DDAIMOS_CMD_PROGRAM=CMD_PROGRAM_$(subst .,_,$(1)) \
	    -DDAIMOS_CMD_TOKEN=$(subst .,_,$(1)) \
	    -S $$< -o $$(@:.dobj=.s)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$(@:.dobj=.s)

$(USERLAND_BUILD)/program/$(1).dobj: exec/standalone.c exec/cmd.h
	mkdir -p $$(dir $$@)
	$$(KCC) $$(KCCFLAGS) $$(CPPFLAGS) \
	    -DDAIMOS_CMD_TOKEN=$(subst .,_,$(1)) \
	    -S $$< -o $$(@:.dobj=.s)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$(@:.dobj=.s)

$(USERLAND_BUILD)/program/$(1).dxr: $(USERLAND_BUILD)/program/$(1).dobj $(COMMAND_S_OBJS) $(COMMAND_LIB)
	$$(DLINK) --daimos-uuo-relax -b 020 -o $$@ -M $$(@:.dxr=.map) $$(COMMAND_S_OBJS) $$< $$(COMMAND_LIB)
endef
$(foreach p,$(COMMAND_PROGRAMS),$(eval $(call COMMAND_PROGRAM_template,$(p))))

define UTILITY_PROGRAM_template
$(USERLAND_BUILD)/utility-member-$(2)/$(1).dobj: exec/utility.c exec/utility.h
	mkdir -p $$(dir $$@)
	$$(KCC) $$(KCCFLAGS) $$(CPPFLAGS) -DUTIL_GROUP=$(4) \
	    -DDAIMOS_UTILITY_PROGRAM=UTILITY_PROGRAM_$(subst .,_,$(1)) \
	    -DDAIMOS_UTILITY_TOKEN=$(subst .,_,$(1)) \
	    -S $$< -o $$(@:.dobj=.s)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$(@:.dobj=.s)

$(USERLAND_BUILD)/program/$(1).dobj: exec/utility_standalone.c exec/utility.h
	mkdir -p $$(dir $$@)
	$$(KCC) $$(KCCFLAGS) $$(CPPFLAGS) \
	    -DDAIMOS_UTILITY_PROGRAM=UTILITY_PROGRAM_$(subst .,_,$(1)) \
	    -DDAIMOS_UTILITY_TOKEN=$(subst .,_,$(1)) \
	    -S $$< -o $$(@:.dobj=.s)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$(@:.dobj=.s)

$(USERLAND_BUILD)/program/$(1).dxr: $(USERLAND_BUILD)/program/$(1).dobj $(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/utility-$(2)/%.dobj) $(UTILITY_$(3)_LIB)
	$$(DLINK) --daimos-uuo-relax -b 020 -o $$@ -M $$(@:.dxr=.map) $$(COMMON_S_SRCS:%.s=$$(USERLAND_BUILD)/utility-$(2)/%.dobj) $$< $$(UTILITY_$(3)_LIB)
endef
$(foreach p,$(UTILITY_MISC_PROGRAMS),$(eval $(call UTILITY_PROGRAM_template,$(p),misc,MISC,1)))
$(foreach p,$(UTILITY_TEXT_PROGRAMS),$(eval $(call UTILITY_PROGRAM_template,$(p),text,TEXT,2)))
$(foreach p,$(UTILITY_DOC_PROGRAMS),$(eval $(call UTILITY_PROGRAM_template,$(p),doc,DOC,3)))

$(USERLAND_BUILD)/utility-misc/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -DUTIL_GROUP=1 -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/utility-misc/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(USERLAND_BUILD)/utility-text/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -DUTIL_GROUP=2 -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/utility-text/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

$(USERLAND_BUILD)/utility-doc/%.dobj: %.c
	mkdir -p $(dir $@)
	$(KCC) $(KCCFLAGS) $(CPPFLAGS) -DUTIL_GROUP=3 -S $< -o $(@:.dobj=.s)
	$(DAS) $(DASFLAGS) -C -O $@ $(@:.dobj=.s)

$(USERLAND_BUILD)/utility-doc/%.dobj: %.s
	mkdir -p $(dir $@)
	$(DAS) $(DASFLAGS) -C -O $@ $<

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

define LOGCOMPAT_PROGRAM_template
$(USERLAND_BUILD)/logcompat-$(1)/%.dobj: %.c
	mkdir -p $$(dir $$@)
	$$(KCC) $$(KCCFLAGS) $$(CPPFLAGS) -DLOGCOMPAT_PROGRAM=$(2) -S $$< -o $$(@:.dobj=.s)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$(@:.dobj=.s)

$(USERLAND_BUILD)/logcompat-$(1)/%.dobj: %.s
	mkdir -p $$(dir $$@)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$<

$(USERLAND_BUILD)/program/$(1).dxr: $$($(3)_C_SRCS:%.c=$(USERLAND_BUILD)/logcompat-$(1)/%.dobj) $$(COMMON_S_SRCS:%.s=$(USERLAND_BUILD)/logcompat-$(1)/%.dobj)
	mkdir -p $$(dir $$@)
	$$(DLINK) --daimos-uuo-relax -b 020 -o $$@ -M $$(@:.dxr=.map) $$(COMMON_S_SRCS:%.s=$$(USERLAND_BUILD)/logcompat-$(1)/%.dobj) $$($(3)_C_SRCS:%.c=$$(USERLAND_BUILD)/logcompat-$(1)/%.dobj)
endef
$(eval $(call LOGCOMPAT_PROGRAM_template,LOGD,1,LOGD))
$(eval $(call LOGCOMPAT_PROGRAM_template,LOGDUMP,2,LOGDUMP))

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

define TOOLCHAIN_PROGRAM_template
$(USERLAND_BUILD)/$(1)/%.dobj: %.c
	mkdir -p $$(dir $$@)
	$$(KCC) $$(KCCFLAGS) $$(CPPFLAGS) -I$$(PROJECT_ROOT)/userland/toolchain -S $$< -o $$(@:.dobj=.s)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$(@:.dobj=.s)

$(USERLAND_BUILD)/$(1)/%.dobj: %.s
	mkdir -p $$(dir $$@)
	$$(DAS) $$(DASFLAGS) -C -O $$@ $$<

$$($(2)_DXR): $$($(2)_C_OBJS) $$($(2)_S_OBJS)
	$$(DLINK) --daimos-uuo-relax -b 020 -o $$@ -M $$($(2)_MAP) $$($(2)_S_OBJS) $$($(2)_C_OBJS)
endef

$(eval $(call TOOLCHAIN_PROGRAM_template,darc,DARC))
$(eval $(call TOOLCHAIN_PROGRAM_template,dlink,DLINK))
$(eval $(call TOOLCHAIN_PROGRAM_template,objdump,OBJDUMP))

clean:
	rm -rf $(USERLAND_BUILD)
