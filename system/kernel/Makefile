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
MKBOOTBANNER ?= ${PDP10_PREFIX}/bin/mkbootbanner
KINIT_DEBUG ?= 0
KINIT_STACK_RESERVE_WORDS ?= 02000
KINIT_STACK_WATERMARK ?= 0
DTFS_TENEX ?= 0
DTFS_ITS ?= 0
PROC_BOOT_USERS ?= 1
PROC_STACK_WATERMARK ?= 0
VM_BACKEND ?= pdp6
VM_BACKEND_C = proc/vm_${VM_BACKEND}.c
VM_BACKEND_H = proc/vm_${VM_BACKEND}.h
VM_BACKEND_ASM = proc/vm_${VM_BACKEND}_asm.s
VM_SWAP_C = proc/vm_${VM_BACKEND}_swap.c
VERSION_FILE = ${PROJECT_ROOT}/VERSION
DAIMON_VERSION != cat ${VERSION_FILE}
INCLUDES = -I. -Iboot -Icore -Idrivers -Ifs -Iipc -Imm -Imodules -Iproc -Istorage -I${PDP10_PREFIX}/include

KERNEL_HEADERS = boot/kboot.h \
	boot/kinit.h \
	core/kcore.h \
	core/kcore_pi.h \
	core/kfmt.h \
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
	fs/d6fs_boot.h \
	fs/d6fs_provider.h \
	fs/monitorfs.h \
	fs/dtfs.h \
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
	storage/diskset.h \
	storage/diskset_boot.h \
	storage/diskset_layout.h \
	storage/diskset_mres.h \
	storage/logstore.h \
	storage/storage.h

KINIT_SRCS = boot/kinit.c \
	boot/kinit_diag.c \
	modules/module_minit.c \
	boot/kfs_boot.c \
	boot/kinit_boot.c \
	fs/d6fs_boot.c \
	fs/d6fs_super_boot.c \
	storage/diskset_boot.c \
	storage/logstore.c \
	proc/proc_boot.c \
	proc/proc_swap_boot.c

KINIT_C_ASM = $(BUILD)/kinit.s \
	$(BUILD)/kinit_diag.s \
	$(BUILD)/module_minit.s \
	$(BUILD)/kfs_boot.s \
	$(BUILD)/kinit_boot.s \
	$(BUILD)/d6fs_boot.s \
	$(BUILD)/d6fs_super_boot.s \
	$(BUILD)/diskset_boot.s \
	$(BUILD)/logstore.s \
	$(BUILD)/proc_boot.s \
	$(BUILD)/proc_swap_boot.s

KINIT_LATE_SRCS = proc/proc_late.c \
	mm/mm_boot.c \
	boot/kinit_late.c

KINIT_LATE_C_ASM = $(BUILD)/proc_late.s \
	$(BUILD)/mm_boot.s \
	$(BUILD)/kinit_late.s

KINIT_FIXED_SRCS = boot/kinit_io.s \
	modules/module_io.s \
	boot/kinit_late_begin.s \
	boot/kinit_late_end.s

KINIT_ASM = ${KINIT_C_ASM} ${KINIT_LATE_C_ASM} ${BUILD}/kinit_io.s ${BUILD}/module_io.s ${BUILD}/bootbanner_generated.s ${BUILD}/kinit_late_begin.s ${BUILD}/kinit_late_end.s

KCORE_C_SRCS = proc/exec.c \
		mm/mm.c \
	modules/module_runtime.c \
	proc/proc.c \
	fs/file.c

KCORE_C_ASM = $(BUILD)/exec.s \
	$(BUILD)/mm.s \
	$(BUILD)/module_runtime.s \
	$(BUILD)/proc.s \
	$(BUILD)/file.s

KCORE_VM_ASM = $(BUILD)/vm_backend.s \
	$(BUILD)/proc_swap.s

KCORE_FIXED_SRCS = core/ret_pdp10.s \
	drivers/dsk270_pdp10.s \
	drivers/drm236_pdp10.s \
	core/kfmt_pdp10.s \
	fs/vfs_pdp10.s \
	fs/fs_mres_pdp10.s \
	fs/file_pdp10.s \
	ipc/pipe_pdp10.s \
		proc/proc_pdp10.s \
	proc/proc_run_pdp10.s \
	drivers/device_state_pdp10.s \
	proc/syscall_info_pdp10.s \
	drivers/pclk_pdp10.s \
	drivers/lpt_io.s \
	proc/syscall_dispatch.s \
	proc/mach_user.s \
	core/kcore_io.s \
	storage/storage_router.s \
	core/kcore_end.s

KCORE_FIXED_ASM = $(BUILD)/exec.s \
	$(BUILD)/mm.s \
	$(BUILD)/module_runtime.s \
	$(BUILD)/proc.s \
	$(BUILD)/file.s \
	$(BUILD)/vm_backend.s \
	$(BUILD)/proc_swap.s \
	$(BUILD)/ret_pdp10.s \
	$(BUILD)/dsk270_pdp10.s \
	$(BUILD)/drm236_pdp10.s \
	$(BUILD)/kfmt_pdp10.s \
	$(BUILD)/vfs_pdp10.s \
	$(BUILD)/fs_mres_pdp10.s \
	$(BUILD)/file_pdp10.s \
	$(BUILD)/pipe_pdp10.s \
	$(BUILD)/monitorfs_pdp10.s \
	$(BUILD)/proc_pdp10.s \
	$(BUILD)/proc_run_pdp10.s \
	$(BUILD)/device_state_pdp10.s \
	$(BUILD)/syscall_info_pdp10.s \
	$(BUILD)/pclk_pdp10.s \
	$(BUILD)/lpt_io.s \
	$(BUILD)/syscall_dispatch.s \
	$(BUILD)/mach_user.s \
	$(BUILD)/vm_backend_asm.s \
	$(BUILD)/kcore_io.s \
	$(BUILD)/storage_router.s \
	$(BUILD)/kcore_end.s

MRES_C_SRCS = fs/d6fs.c \
	fs/d6fs_provider.c

MRES_C_ASM = $(BUILD)/d6fs.s \
	$(BUILD)/d6fs_provider.s

MRES_FIXED_SRCS = fs/d6fs_validate_pdp10.s \
	drivers/cty_io.s \
	drivers/clk_io.s \
	drivers/ptr_io.s \
	drivers/ptp_io.s \
	drivers/cr_io.s \
	drivers/cp_io.s \
	storage/diskset_dispatch.s \
	drivers/dcs_io.s \
	drivers/ge_io.s \
	drivers/dpy_io.s \
	drivers/tty_io.s \
	fs/dtfs_pdp10.s \
	fs/memfs_pdp10.s \
	fs/d6fs_pdp10.s \
	drivers/wcnsls_io.s \
	drivers/ocnsls_io.s \
	drivers/dsk_io.s \
	drivers/drm236_io.s \
	drivers/tape_io.s \
	drivers/slv_io.s

MRES_ASM = $(BUILD)/dtfs.s \
	$(BUILD)/d6fs.s \
	$(BUILD)/d6fs_provider.s \
	$(BUILD)/d6fs_validate_pdp10.s \
	$(BUILD)/cty_io.s \
	$(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s \
	$(BUILD)/ptp_io.s \
	$(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s \
	$(BUILD)/diskset_dispatch.s \
	$(BUILD)/dcs_io.s \
	$(BUILD)/ge_io.s \
	$(BUILD)/dpy_io.s \
	$(BUILD)/tty_io.s \
	$(BUILD)/dtfs_pdp10.s \
	$(BUILD)/memfs_pdp10.s \
	$(BUILD)/d6fs_pdp10.s \
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
	@base=`basename "$@" .s`; src=; for d in boot core drivers fs ipc mm modules proc storage; do if test -f "$$d/$$base.c"; then src="$$d/$$base.c"; break; fi; done; test -n "$$src"; debug=; if test "${KINIT_DEBUG}" = 1; then debug=-DKINIT_DEBUG; fi; ${PDP10_CC} ${PDP10_CFLAGS} -DDAIMON_VERSION_TEXT='"V${DAIMON_VERSION}  "' -DKINIT_STACK_RESERVE_WORDS=${KINIT_STACK_RESERVE_WORDS} -DKINIT_STACK_WATERMARK=${KINIT_STACK_WATERMARK} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} -DPROC_STACK_WATERMARK=${PROC_STACK_WATERMARK} $$debug ${INCLUDES} -S "$$src" -o "$@"

${BUILD}/bootbanner_generated.s: ${VERSION_FILE}
	@mkdir -p ${BUILD}
	${MKBOOTBANNER} -v '${DAIMON_VERSION}' -o $@

${KCORE_C_ASM}: ${KCORE_C_SRCS} ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	@base=`basename "$@" .s`; src=; for d in boot core drivers fs ipc mm modules proc storage; do if test -f "$$d/$$base.c"; then src="$$d/$$base.c"; break; fi; done; test -n "$$src"; ${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} ${INCLUDES} -S "$$src" -o "$@"

${BUILD}/vm_backend.s: ${VM_BACKEND_C} ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} ${INCLUDES} -S ${VM_BACKEND_C} -o $@

${BUILD}/proc_swap.s: ${VM_SWAP_C} ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DPROC_BOOT_USERS=${PROC_BOOT_USERS} ${INCLUDES} -S ${VM_SWAP_C} -o $@

${BUILD}/vm_backend_asm.s: ${VM_BACKEND_ASM}
	@mkdir -p ${BUILD}
	cp ${VM_BACKEND_ASM} $@

${BUILD}/d6fs.s: fs/d6fs.c ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} ${INCLUDES} -S fs/d6fs.c -o $@

${BUILD}/d6fs_provider.s: fs/d6fs_provider.c fs/d6fs_provider_mutation_pdp10.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} ${INCLUDES} -S fs/d6fs_provider.c -o $@.c
	cat $@.c fs/d6fs_provider_mutation_pdp10.s > $@
	rm -f $@.c

${BUILD}/dtfs.s: fs/dtfs.c fs/dtfs_chain_walk_pdp10.s fs/dtfs_resize_pdp10.s ${KERNEL_HEADERS}
	@mkdir -p ${BUILD}
	${PDP10_CC} ${PDP10_CFLAGS} -DDTFS_ENABLE_TENEX=${DTFS_TENEX} -DDTFS_ENABLE_ITS=${DTFS_ITS} ${INCLUDES} -S fs/dtfs.c -o $@.c
	@cat $@.c > $@; if test "${DTFS_TENEX}" = 1 -o "${DTFS_ITS}" = 1; then cat fs/dtfs_chain_walk_pdp10.s fs/dtfs_resize_pdp10.s >> $@; fi; rm -f $@.c

${BUILD}/kinit_io.s: boot/kinit_io.s
	@mkdir -p ${BUILD}
	{ echo ".set KINIT_STACK_WATERMARK,${KINIT_STACK_WATERMARK}"; echo ".set KINIT_STACK_RESERVE_WORDS,${KINIT_STACK_RESERVE_WORDS}"; cat boot/kinit_io.s; } > $@

${BUILD}/proc_pdp10.s: proc/proc_pdp10.s
	@mkdir -p ${BUILD}
	{ echo ".set PROC_STACK_WATERMARK,${PROC_STACK_WATERMARK}"; echo ".set KINIT_STACK_WATERMARK,${KINIT_STACK_WATERMARK}"; cat proc/proc_pdp10.s; } > $@

${BUILD}/proc_run_pdp10.s: proc/proc_run_pdp10.s
	@mkdir -p ${BUILD}
	{ echo ".set PROC_STACK_WATERMARK,${PROC_STACK_WATERMARK}"; cat proc/proc_run_pdp10.s; } > $@

${BUILD}/dtfs_pdp10.s: fs/dtfs_pdp10.s
	@mkdir -p ${BUILD}
	@foreign=0; if test "${DTFS_TENEX}" = 1 -o "${DTFS_ITS}" = 1; then foreign=1; fi; { echo ".set DTFS_ENABLE_TENEX,${DTFS_TENEX}"; echo ".set DTFS_ENABLE_ITS,${DTFS_ITS}"; echo ".set DTFS_ENABLE_FOREIGN,$$foreign"; cat fs/dtfs_pdp10.s; } > $@

COPY_ASM_SRCS = core/ret_pdp10.s \
	drivers/dsk270_pdp10.s \
	drivers/drm236_pdp10.s \
	core/kfmt_pdp10.s \
	fs/vfs_pdp10.s \
	fs/fs_mres_pdp10.s \
	fs/file_pdp10.s \
	ipc/pipe_pdp10.s \
		drivers/device_state_pdp10.s \
	proc/syscall_info_pdp10.s \
	drivers/pclk_pdp10.s \
	drivers/lpt_io.s \
	proc/syscall_dispatch.s \
	proc/mach_user.s \
	core/kcore_io.s \
	storage/storage_router.s \
	core/kcore_end.s \
	fs/d6fs_validate_pdp10.s \
	drivers/cty_io.s \
	drivers/clk_io.s \
	drivers/ptr_io.s \
	drivers/ptp_io.s \
	drivers/cr_io.s \
	drivers/cp_io.s \
	storage/diskset_dispatch.s \
	drivers/dcs_io.s \
	drivers/ge_io.s \
	drivers/dpy_io.s \
	drivers/tty_io.s \
	fs/memfs_pdp10.s \
	fs/d6fs_pdp10.s \
	drivers/wcnsls_io.s \
	drivers/ocnsls_io.s \
	drivers/dsk_io.s \
	drivers/drm236_io.s \
	drivers/tape_io.s \
	drivers/slv_io.s \
	modules/module_io.s \
	boot/kinit_late_begin.s \
	boot/kinit_late_end.s

COPY_ASM_TARGETS = $(BUILD)/ret_pdp10.s \
	$(BUILD)/dsk270_pdp10.s \
	$(BUILD)/drm236_pdp10.s \
	$(BUILD)/kfmt_pdp10.s \
	$(BUILD)/vfs_pdp10.s \
	$(BUILD)/fs_mres_pdp10.s \
	$(BUILD)/file_pdp10.s \
	$(BUILD)/pipe_pdp10.s \
	$(BUILD)/monitorfs_pdp10.s \
	$(BUILD)/device_state_pdp10.s \
	$(BUILD)/syscall_info_pdp10.s \
	$(BUILD)/pclk_pdp10.s \
	$(BUILD)/lpt_io.s \
	$(BUILD)/syscall_dispatch.s \
	$(BUILD)/mach_user.s \
	$(BUILD)/kcore_io.s \
	$(BUILD)/storage_router.s \
	$(BUILD)/kcore_end.s \
	$(BUILD)/d6fs_validate_pdp10.s \
	$(BUILD)/cty_io.s \
	$(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s \
	$(BUILD)/ptp_io.s \
	$(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s \
	$(BUILD)/diskset_dispatch.s \
	$(BUILD)/dcs_io.s \
	$(BUILD)/ge_io.s \
	$(BUILD)/dpy_io.s \
	$(BUILD)/tty_io.s \
	$(BUILD)/memfs_pdp10.s \
	$(BUILD)/d6fs_pdp10.s \
	$(BUILD)/wcnsls_io.s \
	$(BUILD)/ocnsls_io.s \
	$(BUILD)/dsk_io.s \
	$(BUILD)/drm236_io.s \
	$(BUILD)/tape_io.s \
	$(BUILD)/slv_io.s \
	$(BUILD)/module_io.s \
	$(BUILD)/kinit_late_begin.s \
	$(BUILD)/kinit_late_end.s

${COPY_ASM_TARGETS}: ${COPY_ASM_SRCS}
	@mkdir -p ${BUILD}
	@base=`basename "$@"`; src=; for d in boot core drivers fs ipc mm modules proc storage; do if test -f "$$d/$$base"; then src="$$d/$$base"; break; fi; done; test -n "$$src"; cp "$$src" "$@"

clean:
	rm -rf ${BUILD}
