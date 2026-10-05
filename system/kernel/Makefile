# Kernel build.  Keep this makefile within the common GNU/BSD make subset.
PROJECT_ROOT != cd ../.. && pwd
BUILD_ROOT ?= ${PROJECT_ROOT}/build
BUILD ?= ${BUILD_ROOT}/system/kernel
PDP10_PREFIX ?= ${HOME}/cross
PDP10_GCC ?= ${PDP10_PREFIX}/bin/pdp10-dec-none-gcc
# KCC is the normal compiler.  A command-line CC=gcc selects the existing
# PDP-10 GCC driver; any other explicit CC value is used verbatim.
CC = kcc
KCCFLAGS ?= -Pgnu99 -O -x=pdp6 -m=gas
GCCFLAGS ?= -std=gnu99 -Os -march=pdp6 -mcpu=pdp6
CFLAGS ?=
PDP10_CC != cc="${CC}"; case "$$cc" in kcc) echo "${PDP10_PREFIX}/bin/kcc" ;; gcc) echo "${PDP10_GCC}" ;; *) echo "$$cc" ;; esac
PDP10_CFLAGS != cc="${CC}"; case "$$cc" in kcc) echo "${KCCFLAGS}" ;; gcc) echo "${GCCFLAGS}" ;; *) echo "${CFLAGS}" ;; esac
PDP10_CC_DEP != set -- ${PDP10_CC}; command -v "$$1" 2>/dev/null || echo "$$1"
MKBOOTBANNER ?= ${PDP10_PREFIX}/bin/mkbootbanner
KINIT_DEBUG ?= 0
KINIT_STACK_RESERVE_WORDS ?= 02000
KINIT_STACK_WATERMARK ?= 0
KINIT_BADMAP ?= 1
KINIT_FULL ?= 1
PROC_BOOT_USERS ?= 1
PROC_STACK_WATERMARK ?= 0
VM_BACKEND ?= pdp6
VM_BACKEND_C = proc/vm_${VM_BACKEND}.c
VM_BACKEND_H = proc/vm_${VM_BACKEND}.h
VM_BACKEND_FIXED = proc/vm_${VM_BACKEND}.s
VM_SWAP_C = proc/vm_${VM_BACKEND}_swap.c
VERSION_FILE = ${PROJECT_ROOT}/VERSION
DAIMON_VERSION != cat ${VERSION_FILE}
INCLUDES = -I. -Iboot -Icore -Idrivers -Ifs -Iipc -Imm -Imodules -Iproc -Istorage -I${PDP10_PREFIX}/include

KERNEL_HEADERS = boot/kinit.h \
	boot/root_select.h \
	boot/tsfs_boot.h \
	core/kcore.h \
	core/kcore_pi.h \
	drivers/card.h \
	drivers/clk.h \
	drivers/cty.h \
	drivers/dcs.h \
	drivers/dpy.h \
	drivers/dsk270.h \
	drivers/drm236.h \
	drivers/ge.h \
	drivers/ocnsls.h \
	drivers/pt.h \
	drivers/lpt.h \
	drivers/pclk.h \
	drivers/slv.h \
	drivers/tty.h \
	drivers/wcnsls.h \
	fs/d6fs.h \
	boot/d6fs_boot.h \
	fs/d6fs_provider.h \
	fs/monitorfs.h \
	fs/dtfs.h \
	fs/dtfs_media.h \
	fs/tsfs.h \
	fs/file.h \
	fs/fs_mres.h \
	fs/memfs.h \
	fs/vfs.h \
	ipc/pipe.h \
	mm/mm.h \
	mm/mm_internal.h \
	mm/mres.h \
	mm/mres_reloc.h \
	modules/module.h \
	modules/module_runtime.h \
	proc/exec.h \
	proc/mach_user.h \
	proc/proc.h \
	proc/proc_swap.h \
	proc/syscall.h \
	proc/vm.h \
	${VM_BACKEND_H} \
	storage/blockset.h \
	storage/badmap.h \
	storage/blockset_boot.h \
	storage/blockset_layout.h \
	storage/blockset_mres.h \
	storage/auxstore.h \
	storage/bcache.h \
	storage/bstore.h \
	storage/fs_backing.h \
	storage/logstore.h \
	storage/storage.h

KINIT_SRCS = boot/kinit.c \
	boot/kinit_diag.c \
	modules/module_minit.c \
	boot/kinit_boot.c \
	boot/root_select.c \
	boot/tsfs_boot.c \
	boot/d6fs_boot.c \
	boot/d6fs_super_boot.c \
	storage/blockset_boot.c \
	storage/auxstore_boot.c \
	storage/logstore_boot.c \
	storage/logstore.c \
	proc/proc_boot.c \
	proc/proc_swap_boot.c

KINIT_C_ASM = $(BUILD)/kinit.s \
	$(BUILD)/kinit_diag.s \
	$(BUILD)/module_minit.s \
	$(BUILD)/kinit_boot.s \
	$(BUILD)/root_select.s \
	$(BUILD)/tsfs_boot.s \
	$(BUILD)/d6fs_boot.s \
	$(BUILD)/d6fs_super_boot.s \
	$(BUILD)/blockset_boot.s \
	$(BUILD)/auxstore_boot.s \
	$(BUILD)/logstore_boot.s \
	$(BUILD)/logstore.s \
	$(BUILD)/proc_boot.s \
	$(BUILD)/proc_swap_boot.s

KINIT_LATE_SRCS = proc/proc_late.c \
	mm/mm_boot.c \
	boot/kinit_late.c

KINIT_LATE_C_ASM = $(BUILD)/proc_late.s \
	$(BUILD)/mm_boot.s \
	$(BUILD)/kinit_late.s

KINIT_FIXED_SRCS = boot/kinit_io_pdp6.s \
	modules/module_io.s \
	boot/kinit_late_begin_pdp6.s \
	boot/kinit_late_end.s

KINIT_ASM = ${KINIT_C_ASM} ${KINIT_LATE_C_ASM} ${BUILD}/kinit_io_pdp6.s ${BUILD}/module_io.s ${BUILD}/bootbanner_generated.s ${BUILD}/kinit_late_begin_pdp6.s ${BUILD}/kinit_late_end.s

KCORE_C_SRCS = proc/exec.c \
		mm/mm.c \
	proc/proc.c \
	fs/file.c

KCORE_C_ASM = $(BUILD)/mm.s \
	$(BUILD)/proc.s \
	$(BUILD)/file.s

KCORE_VM_ASM = $(BUILD)/vm_backend.s \
	$(BUILD)/proc_swap.s

PDP10_CC_ASM = ${KINIT_C_ASM} ${KINIT_LATE_C_ASM} ${KCORE_C_ASM} \
	${BUILD}/exec.s ${BUILD}/module_runtime.s ${KCORE_VM_ASM} \
	${BUILD}/memfs_data.s ${BUILD}/memfs_snapshot.s \
	${BUILD}/d6fs_provider.s ${BUILD}/dtfs.s ${BUILD}/tsfs.s

${PDP10_CC_ASM}: ${PDP10_CC_DEP}

KCORE_FIXED_SRCS = core/ret.s \
	drivers/dsk270.s \
	drivers/drm236.s \
	core/kfmt_pdp6.s \
	core/s6rec.s \
	fs/vfs.s \
	fs/fs_mres.s \
	fs/tsfs_runtime.s \
	fs/file_runtime.s \
	ipc/pipe.s \
		proc/proc_pdp6.s \
	proc/proc_run.s \
	drivers/device_state.s \
	proc/syscall_info.s \
	drivers/pclk.s \
	drivers/lpt_io.s \
	proc/syscall_dispatch_pdp6.s \
	proc/mach_user_pdp6.s \
	core/kcore_pi_pdp6.s \
	storage/backstore.s \
	storage/bcache_pdp6.s \
	storage/storage_router.s \
	core/kcore_end.s

KCORE_FIXED_ASM = $(BUILD)/d6lz_fixed.s \
	$(BUILD)/exec.s \
	$(BUILD)/d6lz.s \
	$(BUILD)/mm.s \
	$(BUILD)/proc.s \
	$(BUILD)/backstore.s \
	$(BUILD)/file.s \
	$(BUILD)/vm_backend.s \
	$(BUILD)/proc_swap.s \
	$(BUILD)/ret.s \
	$(BUILD)/dsk270.s \
	$(BUILD)/drm236.s \
	$(BUILD)/kfmt_pdp6.s \
	$(BUILD)/s6rec.s \
	$(BUILD)/vfs.s \
	$(BUILD)/fs_mres.s \
	$(BUILD)/tsfs_runtime.s \
	$(BUILD)/file_runtime.s \
	$(BUILD)/pipe.s \
	$(BUILD)/monitorfs_runtime.s \
	$(BUILD)/proc_pdp6.s \
	$(BUILD)/proc_run.s \
	$(BUILD)/device_state.s \
	$(BUILD)/syscall_info.s \
	$(BUILD)/pclk.s \
	$(BUILD)/lpt_io.s \
	$(BUILD)/syscall_dispatch_pdp6.s \
	$(BUILD)/mach_user_pdp6.s \
	$(BUILD)/vm_backend_pdp6.s \
	$(BUILD)/kcore_pi_pdp6.s \
	$(BUILD)/bcache_pdp6.s \
	$(BUILD)/storage_router.s \
	$(BUILD)/kcore_end.s

MRES_C_SRCS = fs/d6fs_provider.c \
	fs/memfs_data.c \
	fs/memfs_snapshot.c \
	fs/tsfs.c

MRES_C_ASM = $(BUILD)/d6fs_provider.s \
	$(BUILD)/dpy_text.s \
	$(BUILD)/memfs_data.s \
	$(BUILD)/memfs_snapshot.s \
	$(BUILD)/tsfs.s

MRES_FIXED_SRCS = fs/d6fs_validate.s \
	drivers/cty_io.s \
	drivers/clk_io.s \
	drivers/ptr_io.s \
	drivers/ptp_io.s \
	drivers/cr_io.s \
	drivers/cp_io.s \
	storage/blockset_dispatch.s \
	storage/badmap_dispatch.s \
	storage/fs_backing.s \
	storage/logstore_runtime.s \
	drivers/dcs_io.s \
	drivers/ge_io.s \
	drivers/dpy_io.s \
	drivers/tty_io.s \
	fs/dtfs_runtime.s \
	fs/tsfs_runtime.s \
	fs/memfs_runtime.s \
	fs/d6fs_runtime.s \
	drivers/wcnsls_io.s \
	drivers/ocnsls_io.s \
	drivers/dsk_io.s \
	drivers/drm236_io.s \
	drivers/tape_io.s \
	drivers/slv_io.s

MRES_ASM = $(BUILD)/dtfs.s \
	$(BUILD)/tsfs.s \
	$(BUILD)/d6fs_provider.s \
	$(BUILD)/dpy_text.s \
	$(BUILD)/memfs_data.s \
	$(BUILD)/memfs_snapshot.s \
	$(BUILD)/d6fs_validate.s \
	$(BUILD)/cty_io.s \
	$(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s \
	$(BUILD)/ptp_io.s \
	$(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s \
	$(BUILD)/blockset_dispatch.s \
	$(BUILD)/badmap_dispatch.s \
	$(BUILD)/fs_backing.s \
	$(BUILD)/logstore_runtime.s \
	$(BUILD)/dcs_io.s \
	$(BUILD)/ge_io.s \
	$(BUILD)/dpy_io.s \
	$(BUILD)/tty_io.s \
	$(BUILD)/dtfs_runtime.s \
	$(BUILD)/tsfs_runtime.s \
	$(BUILD)/memfs_runtime.s \
	$(BUILD)/d6fs_runtime.s \
	$(BUILD)/wcnsls_io.s \
	$(BUILD)/ocnsls_io.s \
	$(BUILD)/dsk_io.s \
	$(BUILD)/drm236_io.s \
	$(BUILD)/tape_io.s \
	$(BUILD)/slv_io.s

.PHONY: kinit kcore clean

kinit: ${KINIT_ASM}

kcore: ${KCORE_FIXED_ASM} ${MRES_ASM}

${KINIT_C_ASM} ${KINIT_LATE_C_ASM}: ${KINIT_SRCS} ${KINIT_LATE_SRCS} ${KERNEL_HEADERS} ${VERSION_FILE}
	@mkdir -p ${BUILD}
	@base=`basename "$@" .s`; src=; for d in boot core drivers fs ipc mm modules proc storage; do if test -f "$$d/$$base.c"; then src="$$d/$$base.c"; break; fi; done; test -n "$$src"; debug=; if test "${KINIT_DEBUG}" = 1; then debug=-DKINIT_DEBUG; fi; ${PDP10_CC} ${PDP10_CFLAGS} -DDAIMON_VERSION_TEXT='"V${DAIMON_VERSION}  "' -DKINIT_STACK_RESERVE_WORDS=${KINIT_STACK_RESERVE_WORDS} -DKINIT_STACK_WATERMARK=${KINIT_STACK_WATERMARK} -DKINIT_BADMAP=${KINIT_BADMAP} -DKINIT_FULL=${KINIT_FULL} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} -DPROC_STACK_WATERMARK=${PROC_STACK_WATERMARK} $$debug ${INCLUDES} -S "$$src" -o "$@"

${BUILD}/bootbanner_generated.s: ${VERSION_FILE} ${MKBOOTBANNER}
	@mkdir -p ${BUILD}
	${MKBOOTBANNER} -v '${DAIMON_VERSION}' -o $@

${KCORE_C_ASM}: ${KCORE_C_SRCS} ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	@base=`basename "$@" .s`; src=; extra=; for d in boot core drivers fs ipc mm modules proc storage; do if test -f "$$d/$$base.c"; then src="$$d/$$base.c"; break; fi; done; test -n "$$src"; if test "$$base" = proc; then extra="-DPROC_EVENT_APPLY_PDP6_ASM=1 -DPROC_SWAP_VICTIM_PDP6_ASM=1 -DPROC_WAIT_STATUS_PDP6_ASM=1 -DPROC_SCHED_SELECT_PDP6_ASM=1 -DPROC_FINISH_SLOT_PDP6_ASM=1 -DPROC_UAREA_RELEASE_PDP6_ASM=1 -DPROC_TTY_RELEASE_PDP6_ASM=1"; elif test "$$base" = file; then extra="-DFILE_OPEN_PDP6_ASM=1"; fi; ${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} $$extra ${INCLUDES} -S "$$src" -o "$@"


${BUILD}/d6lz_fixed.s: ../stand/pdp6/common/decompressor.inc
	@mkdir -p ${BUILD}
	cp $< $@

${BUILD}/exec.s: proc/exec.c proc/exec_lowmem_pdp6.s proc/exec_load.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} -DEXEC_REPLACE_CURRENT_PDP6_ASM=1 ${INCLUDES} -S proc/exec.c -o $@.c
	cat $@.c proc/exec_lowmem_pdp6.s proc/exec_load.s > $@
	rm -f $@.c

${BUILD}/d6lz.s: core/d6lz_pdp6.s
	@mkdir -p ${BUILD}
	cp core/d6lz_pdp6.s $@

${BUILD}/module_runtime.s: modules/module_runtime.c modules/module_runtime_pdp6.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} ${INCLUDES} -S modules/module_runtime.c -o $@.c
	cat $@.c modules/module_runtime_pdp6.s > $@
	rm -f $@.c

${BUILD}/vm_backend.s: ${VM_BACKEND_C} ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	@extra=; if test "${VM_BACKEND}" = pdp6; then extra="-DVM_SPACE_CAN_SWAP_PDP6_ASM=1 -DVM_SPACE_STARTUP_PDP6_ASM=1 -DVM_EXTENT_MOVE_PDP6_ASM=1 -DVM_SPACE_LOAD_FILE_PDP6_ASM=1 -DVM_SPACE_CREATE_PDP6_ASM=1 -DVM_SPACE_DESTROY_PDP6_ASM=1"; fi; ${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} $$extra ${INCLUDES} -S ${VM_BACKEND_C} -o $@

${BUILD}/proc_swap.s: ${VM_SWAP_C} proc/proc_swap_pdp6.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} \
	    -DPROC_SWAP_SERVICE_PDP6_ASM=1 -DPROC_SWAP_RECLAIM_PDP6_ASM=1 \
	    -DPROC_SWAP_TRANSACTIONS_PDP6_ASM=1 \
	    ${INCLUDES} -S ${VM_SWAP_C} -o $@.c
	cat $@.c proc/proc_swap_pdp6.s > $@
	rm -f $@.c

${BUILD}/vm_backend_pdp6.s: ${VM_BACKEND_FIXED}
	@mkdir -p ${BUILD}
	cp ${VM_BACKEND_FIXED} $@

${BUILD}/dpy_text.s: drivers/dpy_text_control_pdp6.s drivers/dpy_text_blocks_pdp6.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	cat drivers/dpy_text_control_pdp6.s drivers/dpy_text_blocks_pdp6.s > $@
${BUILD}/memfs_data.s: fs/memfs_data.c ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} ${INCLUDES} -S fs/memfs_data.c -o $@

${BUILD}/memfs_snapshot.s: fs/memfs_snapshot.c ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} ${INCLUDES} -S fs/memfs_snapshot.c -o $@

${BUILD}/d6fs_provider.s: fs/d6fs_provider.c fs/d6fs_provider_mutation.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} ${INCLUDES} -S fs/d6fs_provider.c -o $@.c
	cat $@.c fs/d6fs_provider_mutation.s > $@
	rm -f $@.c

# PDP-6 production builds use native DTFS only.  TENEX/ITS compatibility is
# disabled to keep the resident DECtape filesystem compact.  The target keeps
# the hand-maintained assembly resize implementation independently.
${BUILD}/dtfs.s: Makefile fs/dtfs.c fs/dtfs_chain_walk.s fs/dtfs_resize.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DDTFS_ENABLE_TENEX=0 -DDTFS_ENABLE_ITS=0 -DDTFS_EXTERNAL_RESIZE=1 ${INCLUDES} -S fs/dtfs.c -o $@.c
	@cat $@.c fs/dtfs_chain_walk.s > $@; { echo ".set DTFS_ENABLE_TENEX,0"; echo ".set DTFS_ENABLE_ITS,0"; cat fs/dtfs_resize.s; } >> $@; rm -f $@.c

${BUILD}/tsfs.s: fs/tsfs.c ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} ${INCLUDES} -S fs/tsfs.c -o $@

${BUILD}/kinit_io_pdp6.s: boot/kinit_io_pdp6.s
	@mkdir -p ${BUILD}
	{ echo ".set KINIT_STACK_WATERMARK,${KINIT_STACK_WATERMARK}"; echo ".set KINIT_STACK_RESERVE_WORDS,${KINIT_STACK_RESERVE_WORDS}"; cat boot/kinit_io_pdp6.s; } > $@

${BUILD}/proc_pdp6.s: proc/proc_pdp6.s
	@mkdir -p ${BUILD}
	{ echo ".set PROC_STACK_WATERMARK,${PROC_STACK_WATERMARK}"; echo ".set KINIT_STACK_WATERMARK,${KINIT_STACK_WATERMARK}"; cat proc/proc_pdp6.s; } > $@

${BUILD}/proc_run.s: proc/proc_run.s
	@mkdir -p ${BUILD}
	{ echo ".set PROC_STACK_WATERMARK,${PROC_STACK_WATERMARK}"; cat proc/proc_run.s; } > $@

${BUILD}/dtfs_runtime.s: Makefile fs/dtfs_runtime.s
	@mkdir -p ${BUILD}
	@{ echo ".set DTFS_ENABLE_TENEX,0"; echo ".set DTFS_ENABLE_ITS,0"; echo ".set DTFS_ENABLE_FOREIGN,0"; cat fs/dtfs_runtime.s; } > $@

COPY_ASM_SRCS = core/ret.s \
	drivers/dsk270.s \
	drivers/drm236.s \
	core/kfmt_pdp6.s \
	core/s6rec.s \
	fs/vfs.s \
	fs/fs_mres.s \
	fs/tsfs_runtime.s \
	fs/file_runtime.s \
	ipc/pipe.s \
		drivers/device_state.s \
	proc/syscall_info.s \
	drivers/pclk.s \
	drivers/lpt_io.s \
	proc/syscall_dispatch_pdp6.s \
	proc/mach_user_pdp6.s \
	core/kcore_pi_pdp6.s \
	storage/backstore.s \
	storage/storage_router.s \
	core/kcore_end.s \
	fs/d6fs_validate.s \
	drivers/cty_io.s \
	drivers/clk_io.s \
	drivers/ptr_io.s \
	drivers/ptp_io.s \
	drivers/cr_io.s \
	drivers/cp_io.s \
	storage/blockset_dispatch.s \
	storage/badmap_dispatch.s \
	storage/fs_backing.s \
	storage/logstore_runtime.s \
	drivers/dcs_io.s \
	drivers/ge_io.s \
	drivers/dpy_io.s \
	drivers/tty_io.s \
	fs/memfs_runtime.s \
	fs/d6fs_runtime.s \
	drivers/wcnsls_io.s \
	drivers/ocnsls_io.s \
	drivers/dsk_io.s \
	drivers/drm236_io.s \
	drivers/tape_io.s \
	drivers/slv_io.s \
	modules/module_io.s \
	boot/kinit_late_begin_pdp6.s \
	boot/kinit_late_end.s

COPY_ASM_TARGETS = $(BUILD)/ret.s \
	$(BUILD)/dsk270.s \
	$(BUILD)/drm236.s \
	$(BUILD)/kfmt_pdp6.s \
	$(BUILD)/s6rec.s \
	$(BUILD)/vfs.s \
	$(BUILD)/fs_mres.s \
	$(BUILD)/tsfs_runtime.s \
	$(BUILD)/file_runtime.s \
	$(BUILD)/pipe.s \
	$(BUILD)/monitorfs_runtime.s \
	$(BUILD)/device_state.s \
	$(BUILD)/syscall_info.s \
	$(BUILD)/pclk.s \
	$(BUILD)/lpt_io.s \
	$(BUILD)/syscall_dispatch_pdp6.s \
	$(BUILD)/mach_user_pdp6.s \
	$(BUILD)/kcore_pi_pdp6.s \
	$(BUILD)/backstore.s \
	$(BUILD)/bcache_pdp6.s \
	$(BUILD)/storage_router.s \
	$(BUILD)/kcore_end.s \
	$(BUILD)/d6fs_validate.s \
	$(BUILD)/cty_io.s \
	$(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s \
	$(BUILD)/ptp_io.s \
	$(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s \
	$(BUILD)/blockset_dispatch.s \
	$(BUILD)/badmap_dispatch.s \
	$(BUILD)/fs_backing.s \
	$(BUILD)/logstore_runtime.s \
	$(BUILD)/dcs_io.s \
	$(BUILD)/ge_io.s \
	$(BUILD)/dpy_io.s \
	$(BUILD)/tty_io.s \
	$(BUILD)/memfs_runtime.s \
	$(BUILD)/d6fs_runtime.s \
	$(BUILD)/wcnsls_io.s \
	$(BUILD)/ocnsls_io.s \
	$(BUILD)/dsk_io.s \
	$(BUILD)/drm236_io.s \
	$(BUILD)/tape_io.s \
	$(BUILD)/slv_io.s \
	$(BUILD)/module_io.s \
	$(BUILD)/kinit_late_begin_pdp6.s \
	$(BUILD)/kinit_late_end.s

${COPY_ASM_TARGETS}: ${COPY_ASM_SRCS}
	@mkdir -p ${BUILD}
	@base=`basename "$@"`; src=; for d in boot core drivers fs ipc mm modules proc storage; do if test -f "$$d/$$base"; then src="$$d/$$base"; break; fi; done; test -n "$$src"; cp "$$src" "$@"

clean:
	rm -rf ${BUILD}
