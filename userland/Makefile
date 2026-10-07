# DAIMOS userland build.
# Keep this makefile in the simple GNU/BSD common subset: ordinary variables,
# explicit/aggregate targets, and portable /bin/sh recipes.  Do not use GNU
# make functions, pattern rules, eval/call metaprogramming, or grouped targets.

USERLAND_DIR != pwd
PROJECT_ROOT != cd .. && pwd
BUILD_ROOT ?= ${PROJECT_ROOT}/build
USERLAND_BUILD = ${BUILD_ROOT}/userland
PDP10_PREFIX ?= ${HOME}/cross
KCC ?= ${PDP10_PREFIX}/bin/kcc
DAS ?= ${PDP10_PREFIX}/bin/das
DASFLAGS ?= -F
DLINK ?= ${PDP10_PREFIX}/bin/dlink
DARC ?= ${PDP10_PREFIX}/bin/darc
P10FOLD ?= ${PDP10_PREFIX}/bin/p10fold
SIXMD_CHECK ?= ${PDP10_PREFIX}/bin/sixmd-check
KCCFLAGS ?= -Pgnu99 -O -x=pdp6 -m=gas
CPPFLAGS = -I${PROJECT_ROOT}/system/kernel/boot -I${PROJECT_ROOT}/system/kernel/core \
	-I${PROJECT_ROOT}/system/kernel/drivers -I${PROJECT_ROOT}/system/kernel/fs \
	-I${PROJECT_ROOT}/system/kernel/mm -I${PROJECT_ROOT}/system/kernel/modules \
	-I${PROJECT_ROOT}/system/kernel/proc -I${PROJECT_ROOT}/system/kernel/storage \
	-I${PROJECT_ROOT}/userland/libc -I${PROJECT_ROOT}/userland/exec \
	-I${PDP10_PREFIX}/include
KCC_HEADERS != find libc exec dsh init login tsfs dtfs d6lz logstore \
	${PROJECT_ROOT}/system/kernel/boot ${PROJECT_ROOT}/system/kernel/core \
	${PROJECT_ROOT}/system/kernel/drivers ${PROJECT_ROOT}/system/kernel/fs \
	${PROJECT_ROOT}/system/kernel/mm ${PROJECT_ROOT}/system/kernel/modules \
	${PROJECT_ROOT}/system/kernel/proc ${PROJECT_ROOT}/system/kernel/storage \
	${PDP10_PREFIX}/include -maxdepth 1 -name '*.h' -print 2>/dev/null

COMMON_C_SRCS = libc/u.c libc/text.c
COMMON_S_SRCS = libc/crt0.s libc/syscall.s libc/syscall_helpers.s

INIT_C_SRCS = libc/u.c libc/text.c libc/logevent.c init/init.c
INIT_S_SRCS = ${COMMON_S_SRCS}
MOUNTALL_C_SRCS = libc/u.c libc/text.c init/mountall.c
MOUNTALL_S_SRCS = ${COMMON_S_SRCS} init/memfs_mount.s
LOGIN_C_SRCS = libc/u.c libc/text.c libc/logevent.c login/login.c
LOGIN_S_SRCS = ${COMMON_S_SRCS}
DSH_C_SRCS = libc/u.c libc/text.c dsh/dsh_s6.c dsh/dsh_lex.c dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c dsh/dsh.c
DSH_S_SRCS = ${COMMON_S_SRCS} dsh/dsh_var_pdp6.s
DSHCOMP_C_SRCS = libc/u.c libc/text.c dsh/dshcomp.c
DSHCOMP_S_SRCS = ${COMMON_S_SRCS}
XARGS_C_SRCS = exec/xargs.c
XARGS_S_SRCS = ${COMMON_S_SRCS}
SED_C_SRCS = libc/u.c libc/text.c exec/sed.c
SED_S_SRCS = ${COMMON_S_SRCS}
CONFIGCTL_C_SRCS = libc/u.c libc/text.c exec/configctl.c
CONFIGCTL_S_SRCS = ${COMMON_S_SRCS}
MAKE_C_SRCS = libc/u.c libc/text.c exec/make.c
MAKE_S_SRCS = ${COMMON_S_SRCS}
TSFSPROBE_C_SRCS = libc/u.c libc/text.c tsfs/tsfs_scan.c tsfs/tsfsprobe.c
TSFSPROBE_S_SRCS = ${COMMON_S_SRCS}
MOUNTTSFS_C_SRCS = libc/u.c libc/text.c tsfs/tsfs_scan.c tsfs/mount_tsfs.c
MOUNTTSFS_S_SRCS = ${COMMON_S_SRCS}
DTFSPROBE_C_SRCS = libc/u.c libc/text.c dtfs/dtfs_probe.c dtfs/dtfsprobe.c
DTFSPROBE_S_SRCS = ${COMMON_S_SRCS}
D6LZ_C_SRCS = libc/u.c d6lz/d6lz.c
D6LZ_S_SRCS = ${COMMON_S_SRCS}
LOGDRAIN_C_SRCS = libc/u.c libc/text.c logstore/logdrain.c
LOGDRAIN_S_SRCS = ${COMMON_S_SRCS}
LOGD_C_SRCS = libc/u.c libc/text.c libc/logevent.c logstore/logcompat.c
LOGDUMP_C_SRCS = libc/u.c libc/text.c logstore/logcompat.c

CHECK_SRCS = libc/u.c libc/text.c exec/commands.c exec/standalone.c \
	exec/utility.c exec/utility_standalone.c exec/xargs.c exec/sed.c \
	exec/configctl.c exec/make.c dsh/dsh.c dsh/dsh_s6.c dsh/dsh_lex.c \
	dsh/dsh_parse.c dsh/dsh_env.c dsh/dsh_exec.c init/init.c login/login.c \
	tsfs/tsfs_scan.c tsfs/tsfsprobe.c tsfs/mount_tsfs.c dtfs/dtfs_probe.c \
	dtfs/dtfsprobe.c d6lz/d6lz.c logstore/logdrain.c logstore/logcompat.c

COMMAND_PROGRAMS = ECHO CAT LS MKDIR RM PWD STAT TOUCH DATE RMDIR CP CHMOD \
	CHOWN MKFS.DTFS FSCK.DTFS MOUNT MOUNT.DTFS UNMOUNT MV HEXDUMP PS DEVS \
	MODS MOUNTS FREE MEMSTAT SYSCTL DF TTYOUT HALT CLEAR DPYVIEW KILL
UTILITY_MISC_PROGRAMS = TRUE FALSE CHECK BASENAME DIRNAME ENV WHICH EXPR BC \
	TEST BRACKET CAL
UTILITY_TEXT_PROGRAMS = CMP HEAD GREP WC TEE TAIL CUT SORT DD STRINGS
UTILITY_DOC_PROGRAMS = MAN HELP APROPOS TREE FIND DU
PUBLIC_MANUAL_NAMES = ${COMMAND_PROGRAMS} ${UTILITY_MISC_PROGRAMS} \
	${UTILITY_TEXT_PROGRAMS} ${UTILITY_DOC_PROGRAMS} LOGD LOGDUMP INIT LOGIN DSH \
	XARGS SED CONFIGCTL MAKE TSFSPROBE MOUNT.TSFS DTFSPROBE D6LZ LOGDRAIN

INIT_DXR = ${USERLAND_BUILD}/init.dxr
MOUNTALL_DXR = ${USERLAND_BUILD}/mountall.dxr
LOGIN_DXR = ${USERLAND_BUILD}/login.dxr
DSH_DXR = ${USERLAND_BUILD}/dsh.dxr
DSHCOMP_DXR = ${USERLAND_BUILD}/dshcomp.dxr
XARGS_DXR = ${USERLAND_BUILD}/xargs.dxr
SED_DXR = ${USERLAND_BUILD}/sed.dxr
CONFIGCTL_DXR = ${USERLAND_BUILD}/configctl.dxr
MAKE_DXR = ${USERLAND_BUILD}/make.dxr
TSFSPROBE_DXR = ${USERLAND_BUILD}/tsfsprobe.dxr
MOUNTTSFS_DXR = ${USERLAND_BUILD}/mounttsfs.dxr
DTFSPROBE_DXR = ${USERLAND_BUILD}/dtfsprobe.dxr
D6LZ_DXR = ${USERLAND_BUILD}/d6lz.dxr
LOGDRAIN_DXR = ${USERLAND_BUILD}/logdrain.dxr
LOGD_DXR = ${USERLAND_BUILD}/program/LOGD.dxr
LOGDUMP_DXR = ${USERLAND_BUILD}/program/LOGDUMP.dxr
COMMAND_LIB = ${USERLAND_BUILD}/libcommand.darc
UTILITY_MISC_LIB = ${USERLAND_BUILD}/libutility-misc.darc
UTILITY_TEXT_LIB = ${USERLAND_BUILD}/libutility-text.darc
UTILITY_DOC_LIB = ${USERLAND_BUILD}/libutility-doc.darc

.PHONY: build check manual-check clean

build: manual-check ${USERLAND_BUILD}/.core.stamp ${USERLAND_BUILD}/.commands.stamp \
	${USERLAND_BUILD}/.utility-misc.stamp ${USERLAND_BUILD}/.utility-text.stamp \
	${USERLAND_BUILD}/.utility-doc.stamp ${USERLAND_BUILD}/.logcompat.stamp

manual-check:
	@set -eu; \
	for n in ${PUBLIC_MANUAL_NAMES}; do \
		f="manual/$$n.SIXMD"; \
		test -f "$$f"; \
		${SIXMD_CHECK} "$$f" "$$n"; \
	done

check: build
	@set -eu; \
	for src in ${CHECK_SRCS}; do \
		rel=$${src%.c}; out="${USERLAND_BUILD}/check/$$rel.s"; \
		mkdir -p "`dirname "$$out"`"; \
		${KCC} ${KCCFLAGS} ${CPPFLAGS} -S "$$src" -o "$$out"; \
	done

# Core programs.  One stamp intentionally owns the small fixed set: this keeps
# dependencies explicit and portable while avoiding dozens of near-identical
# generated rules.
${USERLAND_BUILD}/.core.stamp: Makefile ${INIT_C_SRCS} ${INIT_S_SRCS} \
	${MOUNTALL_C_SRCS} ${MOUNTALL_S_SRCS} ${LOGIN_C_SRCS} ${LOGIN_S_SRCS} \
	${DSH_C_SRCS} ${DSH_S_SRCS} ${DSHCOMP_C_SRCS} ${DSHCOMP_S_SRCS} \
	${XARGS_C_SRCS} ${XARGS_S_SRCS} ${SED_C_SRCS} ${SED_S_SRCS} \
	${CONFIGCTL_C_SRCS} ${CONFIGCTL_S_SRCS} ${MAKE_C_SRCS} ${MAKE_S_SRCS} \
	${TSFSPROBE_C_SRCS} ${TSFSPROBE_S_SRCS} ${MOUNTTSFS_C_SRCS} ${MOUNTTSFS_S_SRCS} \
	${DTFSPROBE_C_SRCS} ${DTFSPROBE_S_SRCS} ${D6LZ_C_SRCS} ${D6LZ_S_SRCS} \
	${LOGDRAIN_C_SRCS} ${LOGDRAIN_S_SRCS} ${KCC_HEADERS} ${KCC}
	@set -eu; \
	compile_c() { cc_group=$$1; cc_src=$$2; shift 2; cc_rel=$${cc_src%.c}; \
		cc_out="${USERLAND_BUILD}/$$cc_group/$$cc_rel.dobj"; cc_asm=$${cc_out%.dobj}.s; \
		mkdir -p "`dirname "$$cc_out"`"; \
		${KCC} ${KCCFLAGS} ${CPPFLAGS} "$$@" -S "$$cc_src" -o "$$cc_asm"; \
		${DAS} ${DASFLAGS} -C -O "$$cc_out" "$$cc_asm"; }; \
	compile_s() { cs_group=$$1; cs_src=$$2; cs_rel=$${cs_src%.s}; cs_out="${USERLAND_BUILD}/$$cs_group/$$cs_rel.dobj"; \
		mkdir -p "`dirname "$$cs_out"`"; ${DAS} ${DASFLAGS} -C -O "$$cs_out" "$$cs_src"; }; \
	build_group() { bg_group=$$1; bg_out=$$2; bg_map=$$3; bg_csrcs=$$4; bg_ssrcs=$$5; bg_extra=$$6; \
		bg_cobjs=; bg_sobjs=; \
		for bg_src in $$bg_csrcs; do compile_c "$$bg_group" "$$bg_src" $$bg_extra; bg_rel=$${bg_src%.c}; bg_cobjs="$$bg_cobjs ${USERLAND_BUILD}/$$bg_group/$$bg_rel.dobj"; done; \
		for bg_src in $$bg_ssrcs; do compile_s "$$bg_group" "$$bg_src"; bg_rel=$${bg_src%.s}; bg_sobjs="$$bg_sobjs ${USERLAND_BUILD}/$$bg_group/$$bg_rel.dobj"; done; \
		${DLINK} --daimos-uuo-relax -b 020 -o "$$bg_out" -M "$$bg_map" $$bg_sobjs $$bg_cobjs; }; \
	mkdir -p "${USERLAND_BUILD}"; \
	build_group init "${INIT_DXR}" "${USERLAND_BUILD}/init.map" "${INIT_C_SRCS}" "${INIT_S_SRCS}" ''; \
	build_group mountall "${MOUNTALL_DXR}" "${USERLAND_BUILD}/mountall.map" "${MOUNTALL_C_SRCS}" "${MOUNTALL_S_SRCS}" ''; \
	build_group login "${LOGIN_DXR}" "${USERLAND_BUILD}/login.map" "${LOGIN_C_SRCS}" "${LOGIN_S_SRCS}" ''; \
	cobjs=; sobjs=; for src in ${DSH_C_SRCS}; do compile_c dsh "$$src"; rel=$${src%.c}; cobjs="$$cobjs ${USERLAND_BUILD}/dsh/$$rel.dobj"; done; \
	for src in ${DSH_S_SRCS}; do compile_s dsh "$$src"; rel=$${src%.s}; sobjs="$$sobjs ${USERLAND_BUILD}/dsh/$$rel.dobj"; done; \
	${P10FOLD} -m 4 -P "${USERLAND_BUILD}/dsh.fold" $$sobjs >/dev/null; \
	${DLINK} --daimos-uuo-relax -b 020 --fold-plan "${USERLAND_BUILD}/dsh.fold" -o "${DSH_DXR}" -M "${USERLAND_BUILD}/dsh.map" $$sobjs $$cobjs; \
	build_group dshcomp "${DSHCOMP_DXR}" "${USERLAND_BUILD}/dshcomp.map" "${DSHCOMP_C_SRCS}" "${DSHCOMP_S_SRCS}" ''; \
	build_group xargs "${XARGS_DXR}" "${USERLAND_BUILD}/xargs.map" "${XARGS_C_SRCS}" "${XARGS_S_SRCS}" ''; \
	build_group sed "${SED_DXR}" "${USERLAND_BUILD}/sed.map" "${SED_C_SRCS}" "${SED_S_SRCS}" ''; \
	build_group configctl "${CONFIGCTL_DXR}" "${USERLAND_BUILD}/configctl.map" "${CONFIGCTL_C_SRCS}" "${CONFIGCTL_S_SRCS}" ''; \
	build_group make "${MAKE_DXR}" "${USERLAND_BUILD}/make.map" "${MAKE_C_SRCS}" "${MAKE_S_SRCS}" ''; \
	build_group tsfsprobe "${TSFSPROBE_DXR}" "${USERLAND_BUILD}/tsfsprobe.map" "${TSFSPROBE_C_SRCS}" "${TSFSPROBE_S_SRCS}" ''; \
	build_group mounttsfs "${MOUNTTSFS_DXR}" "${USERLAND_BUILD}/mounttsfs.map" "${MOUNTTSFS_C_SRCS}" "${MOUNTTSFS_S_SRCS}" ''; \
	build_group dtfsprobe "${DTFSPROBE_DXR}" "${USERLAND_BUILD}/dtfsprobe.map" "${DTFSPROBE_C_SRCS}" "${DTFSPROBE_S_SRCS}" ''; \
	build_group d6lz "${D6LZ_DXR}" "${USERLAND_BUILD}/d6lz.map" "${D6LZ_C_SRCS}" "${D6LZ_S_SRCS}" ''; \
	build_group logdrain "${LOGDRAIN_DXR}" "${USERLAND_BUILD}/logdrain.map" "${LOGDRAIN_C_SRCS}" "${LOGDRAIN_S_SRCS}" ''; \
	touch "$@"

${USERLAND_BUILD}/.commands.stamp: Makefile exec/commands.c exec/standalone.c exec/cmd.h \
	${COMMON_C_SRCS} ${COMMON_S_SRCS} ${KCC_HEADERS} ${KCC}
	@set -eu; \
	mkdir -p "${USERLAND_BUILD}/command-member" "${USERLAND_BUILD}/command" "${USERLAND_BUILD}/program"; \
	common_c=; for src in ${COMMON_C_SRCS}; do rel=$${src%.c}; out="${USERLAND_BUILD}/command/$$rel.dobj"; asm=$${out%.dobj}.s; mkdir -p "`dirname "$$out"`"; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -S "$$src" -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$asm"; common_c="$$common_c $$out"; done; \
	common_s=; for src in ${COMMON_S_SRCS}; do rel=$${src%.s}; out="${USERLAND_BUILD}/command/$$rel.dobj"; mkdir -p "`dirname "$$out"`"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$src"; common_s="$$common_s $$out"; done; \
	members=; for p in ${COMMAND_PROGRAMS}; do tok=`echo "$$p" | tr . _`; out="${USERLAND_BUILD}/command-member/$$p.dobj"; asm=$${out%.dobj}.s; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -DDAIMOS_CMD_PROGRAM=CMD_PROGRAM_$$tok -DDAIMOS_CMD_TOKEN=$$tok -S exec/commands.c -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$asm"; members="$$members $$out"; done; \
	${DARC} -o "${COMMAND_LIB}" $$members $$common_c; \
	for p in ${COMMAND_PROGRAMS}; do tok=`echo "$$p" | tr . _`; obj="${USERLAND_BUILD}/program/$$p.dobj"; asm=$${obj%.dobj}.s; dxr="${USERLAND_BUILD}/program/$$p.dxr"; map=$${dxr%.dxr}.map; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -DDAIMOS_CMD_TOKEN=$$tok -S exec/standalone.c -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$obj" "$$asm"; ${DLINK} --daimos-uuo-relax -b 020 -o "$$dxr" -M "$$map" $$common_s "$$obj" "${COMMAND_LIB}"; done; \
	touch "$@"

${USERLAND_BUILD}/.utility-misc.stamp: Makefile exec/utility.c exec/utility_standalone.c exec/utility.h ${COMMON_C_SRCS} ${COMMON_S_SRCS} ${KCC_HEADERS} ${KCC}
	@${MAKE} utility-group PROGRAMS='${UTILITY_MISC_PROGRAMS}' GROUP=misc GROUPNO=1 LIB='${UTILITY_MISC_LIB}' STAMP='$@'

${USERLAND_BUILD}/.utility-text.stamp: Makefile exec/utility.c exec/utility_standalone.c exec/utility.h ${COMMON_C_SRCS} ${COMMON_S_SRCS} ${KCC_HEADERS} ${KCC}
	@${MAKE} utility-group PROGRAMS='${UTILITY_TEXT_PROGRAMS}' GROUP=text GROUPNO=2 LIB='${UTILITY_TEXT_LIB}' STAMP='$@'

${USERLAND_BUILD}/.utility-doc.stamp: Makefile exec/utility.c exec/utility_standalone.c exec/utility.h ${COMMON_C_SRCS} ${COMMON_S_SRCS} ${KCC_HEADERS} ${KCC}
	@${MAKE} utility-group PROGRAMS='${UTILITY_DOC_PROGRAMS}' GROUP=doc GROUPNO=3 LIB='${UTILITY_DOC_LIB}' STAMP='$@'

.PHONY: utility-group
utility-group:
	@set -eu; \
	mkdir -p "${USERLAND_BUILD}/utility-member-${GROUP}" "${USERLAND_BUILD}/utility-${GROUP}" "${USERLAND_BUILD}/program"; \
	common_c=; for src in ${COMMON_C_SRCS}; do rel=$${src%.c}; out="${USERLAND_BUILD}/utility-${GROUP}/$$rel.dobj"; asm=$${out%.dobj}.s; mkdir -p "`dirname "$$out"`"; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -DUTIL_GROUP=${GROUPNO} -S "$$src" -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$asm"; common_c="$$common_c $$out"; done; \
	common_s=; for src in ${COMMON_S_SRCS}; do rel=$${src%.s}; out="${USERLAND_BUILD}/utility-${GROUP}/$$rel.dobj"; mkdir -p "`dirname "$$out"`"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$src"; common_s="$$common_s $$out"; done; \
	members=; for p in ${PROGRAMS}; do tok=`echo "$$p" | tr . _`; out="${USERLAND_BUILD}/utility-member-${GROUP}/$$p.dobj"; asm=$${out%.dobj}.s; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -DUTIL_GROUP=${GROUPNO} -DDAIMOS_UTILITY_PROGRAM=UTILITY_PROGRAM_$$tok -DDAIMOS_UTILITY_TOKEN=$$tok -S exec/utility.c -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$asm"; members="$$members $$out"; done; \
	${DARC} -o "${LIB}" $$members $$common_c; \
	for p in ${PROGRAMS}; do tok=`echo "$$p" | tr . _`; obj="${USERLAND_BUILD}/program/$$p.dobj"; asm=$${obj%.dobj}.s; dxr="${USERLAND_BUILD}/program/$$p.dxr"; map=$${dxr%.dxr}.map; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -DDAIMOS_UTILITY_PROGRAM=UTILITY_PROGRAM_$$tok -DDAIMOS_UTILITY_TOKEN=$$tok -S exec/utility_standalone.c -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$obj" "$$asm"; ${DLINK} --daimos-uuo-relax -b 020 -o "$$dxr" -M "$$map" $$common_s "$$obj" "${LIB}"; done; \
	touch "${STAMP}"

${USERLAND_BUILD}/.logcompat.stamp: Makefile ${LOGD_C_SRCS} ${LOGDUMP_C_SRCS} ${COMMON_S_SRCS} ${KCC_HEADERS} ${KCC}
	@set -eu; \
	build_log() { name=$$1; mode=$$2; shift 2; csrcs="$$*"; group="logcompat-$$name"; cobjs=; sobjs=; mkdir -p "${USERLAND_BUILD}/$$group" "${USERLAND_BUILD}/program"; for src in $$csrcs; do rel=$${src%.c}; out="${USERLAND_BUILD}/$$group/$$rel.dobj"; asm=$${out%.dobj}.s; mkdir -p "`dirname "$$out"`"; ${KCC} ${KCCFLAGS} ${CPPFLAGS} -DLOGCOMPAT_PROGRAM=$$mode -S "$$src" -o "$$asm"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$asm"; cobjs="$$cobjs $$out"; done; for src in ${COMMON_S_SRCS}; do rel=$${src%.s}; out="${USERLAND_BUILD}/$$group/$$rel.dobj"; mkdir -p "`dirname "$$out"`"; ${DAS} ${DASFLAGS} -C -O "$$out" "$$src"; sobjs="$$sobjs $$out"; done; dxr="${USERLAND_BUILD}/program/$$name.dxr"; ${DLINK} --daimos-uuo-relax -b 020 -o "$$dxr" -M "$${dxr%.dxr}.map" $$sobjs $$cobjs; }; \
	build_log LOGD 1 ${LOGD_C_SRCS}; \
	build_log LOGDUMP 2 ${LOGDUMP_C_SRCS}; \
	touch "$@"

clean:
	rm -rf ${USERLAND_BUILD}
