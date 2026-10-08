.PHONY: force-minit-config check-config
PROJECT_ROOT != cd ../../.. && pwd
KERNEL_DIR = ${PROJECT_ROOT}/system/kernel
PICTURE_DIR ?= ${PROJECT_ROOT}/pictures
BUILD_ROOT ?= ${PROJECT_ROOT}/build
BUILD ?= ${BUILD_ROOT}/system/boot/pdp6
PDP10_PREFIX ?= ${HOME}/cross
DAS_REPO ?= ${PROJECT_ROOT}/../das
DAIMOS_TOOLS_REPO ?= ${PROJECT_ROOT}/../daimos-tools
KCC_REPO ?= ${PROJECT_ROOT}/../kcc

DAS ?= ${PDP10_PREFIX}/bin/das
DASFLAGS ?= -F
DLINK ?= ${PDP10_PREFIX}/bin/dlink
P10FOLD ?= ${PDP10_PREFIX}/bin/p10fold
DXRCONVERT ?= ${PDP10_PREFIX}/bin/dxrconvert
MKSTREAM ?= ${PDP10_PREFIX}/bin/mkstream
D6LZ_HOST ?= ${PDP10_PREFIX}/bin/d6lz
DXRCHECK ?= ${PDP10_PREFIX}/bin/dxrcheck
MKDSK ?= ${PDP10_PREFIX}/bin/mkdsk
MKD6FS ?= ${PDP10_PREFIX}/bin/mkd6fs
MKAUXSTORE ?= ${PDP10_PREFIX}/bin/mkauxstore
MKDT ?= ${PDP10_PREFIX}/bin/mkdt
MKTSFS ?= ${PDP10_PREFIX}/bin/mktsfs
MKTAP ?= ${PDP10_PREFIX}/bin/mktap
WORDS2PT ?= ${PDP10_PREFIX}/bin/words2pt
S6FILTER ?= ${PDP10_PREFIX}/bin/s6filter
SIXMD_CHECK ?= ${PDP10_PREFIX}/bin/sixmd-check
S6TEXT ?= ${PDP10_PREFIX}/bin/s6text
LOGSTORE ?= ${PDP10_PREFIX}/bin/logstore
SIMH_PDP6 ?= ${PDP10_PREFIX}/bin/pdp6
ROOT ?= auto
ROOTSET ?= 1
BOOT ?= auto
BOOT_PROFILE ?= full
DPY_LOGIN ?= 0
KINIT_STACK_RESERVE_WORDS ?= 02000
BOOT_CORE_WORDS_DEFAULT != if test "${BOOT_PROFILE}" = lowmem; then echo 0100000; else echo 01000000; fi
SIMH_CPU_KWORDS_DEFAULT != if test "${BOOT_PROFILE}" = lowmem; then echo 32; else echo 256; fi
KINIT_BADMAP_DEFAULT != if test "${BOOT_PROFILE}" = lowmem; then echo 0; else echo 1; fi
BOOT_CORE_WORDS ?= ${BOOT_CORE_WORDS_DEFAULT}
SIMH_CPU_KWORDS ?= ${SIMH_CPU_KWORDS_DEFAULT}
KINIT_BADMAP ?= ${KINIT_BADMAP_DEFAULT}
KINIT_STACK_WATERMARK ?= 0
PDP10_GCC ?= ${PDP10_PREFIX}/bin/pdp10-dec-none-gcc
PDP10_OBJDUMP ?= ${PDP10_PREFIX}/bin/pdp10-objdump
PDP10_KCC ?= ${PDP10_PREFIX}/bin/kcc

D6FS_LOGSTORE_BLOCKS ?= 0400
D6FS_SWAP_TAIL_BLOCKS ?= 0400
D6FS_SPARE_BLOCKS ?= 0
DRUM_LOGSTORE_BLOCKS ?= 0100
DRUM_BACKSTORE_BLOCKS ?= 0300
PROC_BOOT_USERS ?= 1
PROC_STACK_WATERMARK ?= 0
PROC_SWAP_VERIFY_PURE ?= 0
VM_BACKEND ?= pdp6

KINIT_BUILD = ${BUILD}/kinit-asm
KCORE_BUILD = ${BUILD}/kcore-asm
KINIT_STAMP = ${BUILD}/.kinit-built
KCORE_STAMP = ${BUILD}/.kcore-built
KERNEL_INPUTS = ${KERNEL_DIR}/Makefile \
	${PROJECT_ROOT}/system/stand/pdp6/common/decompressor.inc \
	${KERNEL_DIR}/boot/kinit.c \
	${KERNEL_DIR}/boot/kinit.h \
	${KERNEL_DIR}/boot/kinit_boot.c \
	${KERNEL_DIR}/boot/kinit_diag.c \
	${KERNEL_DIR}/boot/kinit_io_pdp6.s \
	${KERNEL_DIR}/boot/kinit_late.c \
	${KERNEL_DIR}/boot/kinit_late_begin_pdp6.s \
	${KERNEL_DIR}/boot/kinit_late_end.s \
	${KERNEL_DIR}/boot/root_select.c \
	${KERNEL_DIR}/boot/root_select.h \
	${KERNEL_DIR}/core/d6lz.h \
	${KERNEL_DIR}/core/d6lz_pdp6.s \
	${KERNEL_DIR}/core/kcore.h \
	${KERNEL_DIR}/core/kcore_end.s \
	${KERNEL_DIR}/core/kcore_pi_pdp6.s \
	${KERNEL_DIR}/core/kcore_pi.h \
	${KERNEL_DIR}/core/kfmt_pdp6.s \
	${KERNEL_DIR}/core/ret.s \
	${KERNEL_DIR}/drivers/card.h \
	${KERNEL_DIR}/drivers/clk.h \
	${KERNEL_DIR}/drivers/clk_io.s \
	${KERNEL_DIR}/drivers/cp_io.s \
	${KERNEL_DIR}/drivers/cr_io.s \
	${KERNEL_DIR}/drivers/cty.h \
	${KERNEL_DIR}/drivers/cty_io.s \
	${KERNEL_DIR}/drivers/dcs.h \
	${KERNEL_DIR}/drivers/dcs_io.s \
	${KERNEL_DIR}/drivers/device_state.s \
	${KERNEL_DIR}/drivers/dpy.h \
	${KERNEL_DIR}/drivers/dpy_text_control_pdp6.s \
	${KERNEL_DIR}/drivers/dpy_text_blocks_pdp6.s \
	${KERNEL_DIR}/drivers/dpy_io.s \
	${KERNEL_DIR}/drivers/dsk270.h \
	${KERNEL_DIR}/drivers/drm236.h \
	${KERNEL_DIR}/drivers/dsk270.s \
	${KERNEL_DIR}/drivers/drm236.s \
	${KERNEL_DIR}/drivers/dsk_io.s \
	${KERNEL_DIR}/drivers/drm236_io.s \
	${KERNEL_DIR}/drivers/ge.h \
	${KERNEL_DIR}/drivers/ge_io.s \
	${KERNEL_DIR}/drivers/lpt.h \
	${KERNEL_DIR}/drivers/lpt_io.s \
	${KERNEL_DIR}/drivers/ocnsls.h \
	${KERNEL_DIR}/drivers/ocnsls_io.s \
	${KERNEL_DIR}/drivers/pt.h \
	${KERNEL_DIR}/drivers/pclk.h \
	${KERNEL_DIR}/drivers/ptp_io.s \
	${KERNEL_DIR}/drivers/ptr_io.s \
	${KERNEL_DIR}/drivers/slv.h \
	${KERNEL_DIR}/drivers/slv_io.s \
	${KERNEL_DIR}/drivers/tape_io.s \
	${KERNEL_DIR}/drivers/tty.h \
	${KERNEL_DIR}/drivers/tty_io.s \
	${KERNEL_DIR}/drivers/wcnsls.h \
	${KERNEL_DIR}/drivers/wcnsls_io.s \
	${KERNEL_DIR}/fs/d6fs.h \
	${KERNEL_DIR}/boot/d6fs_boot.c \
	${KERNEL_DIR}/boot/d6fs_boot.h \
	${KERNEL_DIR}/fs/d6fs_runtime.s \
	${KERNEL_DIR}/fs/d6fs_provider.c \
	${KERNEL_DIR}/fs/d6fs_provider.h \
	${KERNEL_DIR}/fs/d6fs_provider_mutation.s \
	${KERNEL_DIR}/boot/d6fs_super_boot.c \
	${KERNEL_DIR}/fs/d6fs_validate.s \
	${KERNEL_DIR}/fs/monitorfs.h \
	${KERNEL_DIR}/fs/dtfs.c \
	${KERNEL_DIR}/fs/tsfs.c \
	${KERNEL_DIR}/fs/dtfs.h \
	${KERNEL_DIR}/fs/dtfs_media.h \
	${KERNEL_DIR}/fs/dtfs_chain_walk.s \
	${KERNEL_DIR}/fs/dtfs_runtime.s \
	${KERNEL_DIR}/fs/tsfs_runtime.s \
	${KERNEL_DIR}/fs/dtfs_resize.s \
	${KERNEL_DIR}/fs/file.c \
	${KERNEL_DIR}/fs/file.h \
	${KERNEL_DIR}/fs/file_runtime.s \
	${KERNEL_DIR}/fs/monitorfs_runtime.s \
	${KERNEL_DIR}/fs/fs_mres.h \
	${KERNEL_DIR}/fs/fs_mres.s \
	${KERNEL_DIR}/fs/memfs.c \
	${KERNEL_DIR}/fs/memfs_data.c \
	${KERNEL_DIR}/fs/memfs_snapshot.c \
	${KERNEL_DIR}/fs/memfs.h \
	${KERNEL_DIR}/fs/memfs_runtime.s \
	${KERNEL_DIR}/fs/vfs.h \
	${KERNEL_DIR}/fs/vfs.s \
	${KERNEL_DIR}/ipc/pipe.h \
	${KERNEL_DIR}/ipc/pipe.s \
	${KERNEL_DIR}/mm/mm.c \
	${KERNEL_DIR}/mm/mm.h \
	${KERNEL_DIR}/mm/mm_boot.c \
	${KERNEL_DIR}/mm/mm_internal.h \
	${KERNEL_DIR}/mm/mres.h \
	${KERNEL_DIR}/mm/mres_reloc.h \
	${KERNEL_DIR}/modules/module.h \
	${KERNEL_DIR}/modules/module_io.s \
	${KERNEL_DIR}/modules/module_minit.c \
	${KERNEL_DIR}/modules/module_runtime.c \
	${KERNEL_DIR}/modules/module_runtime.h \
	${KERNEL_DIR}/modules/module_table.s \
	${KERNEL_DIR}/proc/exec.c \
	${KERNEL_DIR}/proc/exec.h \
	${KERNEL_DIR}/proc/exec_lowmem_pdp6.s \
	${KERNEL_DIR}/proc/mach_user.h \
	${KERNEL_DIR}/proc/mach_user_pdp6.s \
	${KERNEL_DIR}/proc/proc.c \
	${KERNEL_DIR}/proc/proc.h \
	${KERNEL_DIR}/proc/proc_boot.c \
	${KERNEL_DIR}/proc/proc_late.c \
	${KERNEL_DIR}/proc/proc_pdp6.s \
	${KERNEL_DIR}/proc/proc_run.s \
	${KERNEL_DIR}/proc/proc_swap.h \
	${KERNEL_DIR}/proc/proc_swap_boot.c \
	${KERNEL_DIR}/proc/proc_swap_pdp6.s \
	${KERNEL_DIR}/proc/syscall.h \
	${KERNEL_DIR}/proc/syscall_dispatch_pdp6.s \
	${KERNEL_DIR}/proc/vm.h \
	${KERNEL_DIR}/proc/vm_${VM_BACKEND}.h \
	${KERNEL_DIR}/proc/vm_${VM_BACKEND}.c \
	${KERNEL_DIR}/proc/vm_${VM_BACKEND}_swap.c \
	${KERNEL_DIR}/proc/vm_${VM_BACKEND}.s \
	${KERNEL_DIR}/proc/syscall_info.s \
	${KERNEL_DIR}/drivers/pclk.s \
	${KERNEL_DIR}/storage/blockset.h \
	${KERNEL_DIR}/storage/blockset_boot.c \
	${KERNEL_DIR}/storage/blockset_boot.h \
	${KERNEL_DIR}/storage/blockset_dispatch.s \
	${KERNEL_DIR}/storage/badmap_dispatch.s \
	${KERNEL_DIR}/storage/blockset_layout.h \
	${KERNEL_DIR}/storage/blockset_mres.h \
	${KERNEL_DIR}/storage/bcache.h \
	${KERNEL_DIR}/storage/bcache_pdp6.s \
	${KERNEL_DIR}/storage/bstore.h \
	${KERNEL_DIR}/storage/backstore.s \
	${KERNEL_DIR}/storage/auxstore_boot.c \
	${KERNEL_DIR}/storage/auxstore.h \
	${KERNEL_DIR}/storage/logstore.c \
	${KERNEL_DIR}/storage/logstore_boot.c \
	${KERNEL_DIR}/storage/logstore.h \
	${KERNEL_DIR}/storage/logstore_drain.c \
	${KERNEL_DIR}/storage/logstore_mtc_sink.c \
	${KERNEL_DIR}/storage/storage.h \
	${KERNEL_DIR}/storage/storage_router.s

KINIT_NAMES = kinit kinit_diag module_minit kinit_boot root_select d6fs_boot d6fs_super_boot blockset_boot auxstore_boot logstore_boot logstore proc_boot proc_swap_boot kinit_io_pdp6 module_io bootbanner_generated
KINIT_LATE_NAMES = kinit_late_begin_pdp6 proc_late mm_boot kinit_late kinit_late_end
KINIT_ASM = ${KINIT_BUILD}/kinit.s ${KINIT_BUILD}/kinit_diag.s ${KINIT_BUILD}/module_minit.s ${KINIT_BUILD}/kinit_boot.s ${KINIT_BUILD}/root_select.s ${KINIT_BUILD}/d6fs_boot.s ${KINIT_BUILD}/d6fs_super_boot.s ${KINIT_BUILD}/blockset_boot.s ${KINIT_BUILD}/auxstore_boot.s ${KINIT_BUILD}/logstore_boot.s ${KINIT_BUILD}/proc_boot.s ${KINIT_BUILD}/proc_swap_boot.s ${KINIT_BUILD}/kinit_io_pdp6.s ${KINIT_BUILD}/module_io.s ${KINIT_BUILD}/bootbanner_generated.s
KINIT_LATE_ASM = ${KINIT_BUILD}/kinit_late_begin_pdp6.s ${KINIT_BUILD}/proc_late.s ${KINIT_BUILD}/mm_boot.s ${KINIT_BUILD}/kinit_late.s ${KINIT_BUILD}/kinit_late_end.s
KINIT_OBJS = ${BUILD}/kinit.dobj ${BUILD}/kinit_diag.dobj ${BUILD}/module_minit.dobj ${BUILD}/kinit_boot.dobj ${BUILD}/root_select.dobj ${BUILD}/tsfs_boot.dobj ${BUILD}/d6fs_boot.dobj ${BUILD}/d6fs_super_boot.dobj ${BUILD}/blockset_boot.dobj ${BUILD}/auxstore_boot.dobj ${BUILD}/logstore_boot.dobj ${BUILD}/logstore.dobj ${BUILD}/proc_boot.dobj ${BUILD}/proc_swap_boot.dobj ${BUILD}/kinit_io_pdp6.dobj ${BUILD}/module_io.dobj ${BUILD}/bootbanner_generated.dobj
KINIT_LATE_OBJS = ${BUILD}/kinit_late_begin_pdp6.dobj ${BUILD}/proc_late.dobj ${BUILD}/mm_boot.dobj ${BUILD}/kinit_late.dobj ${BUILD}/kinit_late_end.dobj

KCORE_C_NAMES = exec d6lz vfs mm proc vm_backend proc_swap file
KCORE_FIXED_NAMES = ${KCORE_C_NAMES} backstore ret \
	dsk270 drm236 kfmt_pdp6 s6rec vfs fs_mres file_runtime monitorfs_runtime proc_pdp6 proc_run \
	device_state syscall_info pclk \
	syscall_dispatch_pdp6 mach_user_pdp6 vm_backend_pdp6 kcore_pi_pdp6 bcache_pdp6 storage_router kcore_end
KCORE_FIXED_ASM = ${KCORE_BUILD}/d6lz_fixed.s ${KCORE_BUILD}/exec.s ${KCORE_BUILD}/d6lz.s ${KCORE_BUILD}/mm.s ${KCORE_BUILD}/proc.s ${KCORE_BUILD}/backstore.s ${KCORE_BUILD}/vm_backend.s ${KCORE_BUILD}/proc_swap.s ${KCORE_BUILD}/file.s ${KCORE_BUILD}/ret.s ${KCORE_BUILD}/dsk270.s ${KCORE_BUILD}/drm236.s ${KCORE_BUILD}/kfmt_pdp6.s ${KCORE_BUILD}/s6rec.s ${KCORE_BUILD}/vfs.s ${KCORE_BUILD}/fs_mres.s ${KCORE_BUILD}/file_runtime.s ${KCORE_BUILD}/pipe.s ${KCORE_BUILD}/monitorfs_runtime.s ${KCORE_BUILD}/proc_pdp6.s ${KCORE_BUILD}/proc_run.s ${KCORE_BUILD}/device_state.s ${KCORE_BUILD}/syscall_info.s ${KCORE_BUILD}/pclk.s ${KCORE_BUILD}/syscall_dispatch_pdp6.s ${KCORE_BUILD}/mach_user_pdp6.s ${KCORE_BUILD}/vm_backend_pdp6.s ${KCORE_BUILD}/kcore_pi_pdp6.s ${KCORE_BUILD}/bcache_pdp6.s ${KCORE_BUILD}/storage_router.s ${KCORE_BUILD}/kcore_end.s
KCORE_FIXED_OBJS = ${BUILD}/d6lz_fixed.dobj ${BUILD}/exec.dobj ${BUILD}/d6lz.dobj ${BUILD}/mm.dobj ${BUILD}/proc.dobj ${BUILD}/backstore.dobj ${BUILD}/vm_backend.dobj ${BUILD}/proc_swap.dobj ${BUILD}/file.dobj ${BUILD}/ret.dobj ${BUILD}/dsk270.dobj ${BUILD}/drm236.dobj ${BUILD}/kfmt_pdp6.dobj ${BUILD}/s6rec.dobj ${BUILD}/vfs.dobj ${BUILD}/fs_mres.dobj ${BUILD}/file_runtime.dobj ${BUILD}/pipe.dobj ${BUILD}/monitorfs_runtime.dobj ${BUILD}/proc_pdp6.dobj ${BUILD}/proc_run.dobj ${BUILD}/device_state.dobj ${BUILD}/syscall_info.dobj ${BUILD}/pclk.dobj ${BUILD}/syscall_dispatch_pdp6.dobj ${BUILD}/mach_user_pdp6.dobj ${BUILD}/vm_backend_pdp6.dobj ${BUILD}/kcore_pi_pdp6.dobj ${BUILD}/bcache_pdp6.dobj ${BUILD}/storage_router.dobj ${BUILD}/kcore_end.dobj
MRES_NAMES = cty_io clk_io ptr_io ptp_io lpt_io cr_io cp_io dcs_io ge_io dpy_io \
	dpy_text tty_io wcnsls_io ocnsls_io dsk_io drm236_io tape_io slv_io memfs_runtime \
	dtfs dtfs_runtime tsfs tsfs_runtime blockset_dispatch badmap_dispatch fs_backing d6fs_provider d6fs_validate d6fs_runtime
MRES_ASM = ${KCORE_BUILD}/cty_io.s ${KCORE_BUILD}/clk_io.s ${KCORE_BUILD}/ptr_io.s ${KCORE_BUILD}/ptp_io.s ${KCORE_BUILD}/lpt_io.s ${KCORE_BUILD}/cr_io.s ${KCORE_BUILD}/cp_io.s ${KCORE_BUILD}/dcs_io.s ${KCORE_BUILD}/ge_io.s ${KCORE_BUILD}/dpy_io.s ${KCORE_BUILD}/dpy_text.s ${KCORE_BUILD}/tty_io.s ${KCORE_BUILD}/wcnsls_io.s ${KCORE_BUILD}/ocnsls_io.s ${KCORE_BUILD}/dsk_io.s ${KCORE_BUILD}/drm236_io.s ${KCORE_BUILD}/tape_io.s ${KCORE_BUILD}/slv_io.s ${KCORE_BUILD}/memfs_data.s ${KCORE_BUILD}/memfs_snapshot.s ${KCORE_BUILD}/memfs_runtime.s ${KCORE_BUILD}/dtfs.s ${KCORE_BUILD}/dtfs_runtime.s ${KCORE_BUILD}/tsfs.s ${KCORE_BUILD}/tsfs_runtime.s ${KCORE_BUILD}/blockset_dispatch.s ${KCORE_BUILD}/badmap_dispatch.s ${KCORE_BUILD}/fs_backing.s ${KCORE_BUILD}/logstore_runtime.s ${KCORE_BUILD}/d6fs_provider.s ${KCORE_BUILD}/d6fs_validate.s ${KCORE_BUILD}/d6fs_runtime.s

KCORE_DXR = ${BUILD}/kcore.dxr
KCORE_MAP = ${BUILD}/kcore.map
KCORE_FOLD_PLAN = ${BUILD}/kcore.fold
# Pre-self-hosting fold scope: all handwritten KCORE assembler.  Generated
# KCC output remains deliberately excluded until self-hosting/code-generator
# canonicalization work.  P10FOLD selects only relocation-safe zero-cost donors.
KCORE_FOLD_OBJS = ${BUILD}/ret.dobj \
	${BUILD}/dsk270.dobj ${BUILD}/drm236.dobj \
	${BUILD}/kfmt_pdp6.dobj ${BUILD}/s6rec.dobj ${BUILD}/vfs.dobj \
	${BUILD}/fs_mres.dobj ${BUILD}/file_runtime.dobj \
	${BUILD}/pipe.dobj ${BUILD}/monitorfs_runtime.dobj \
	${BUILD}/proc_pdp6.dobj ${BUILD}/proc_run.dobj \
	${BUILD}/device_state.dobj ${BUILD}/syscall_info.dobj \
	${BUILD}/pclk.dobj \
	${BUILD}/syscall_dispatch_pdp6.dobj ${BUILD}/mach_user_pdp6.dobj \
	${BUILD}/kcore_pi_pdp6.dobj ${BUILD}/bcache_pdp6.dobj ${BUILD}/storage_router.dobj ${BUILD}/kcore_end.dobj
KCORE_RIM = ${BUILD}/kcore.rim
KCORE_STREAM = ${BUILD}/kcore.stream
KCORE_EMBED_ASM = ${BUILD}/kcore-embed.s
KCORE_EMBED_OBJ = ${BUILD}/kcore-embed.dobj

CTY_MRES_OBJ = ${BUILD}/cty-mres.dobj
CLK_MRES_OBJ = ${BUILD}/clk-mres.dobj
PTR_MRES_OBJ = ${BUILD}/ptr-mres.dobj
PTP_MRES_OBJ = ${BUILD}/ptp-mres.dobj
LPT_MRES_OBJ = ${BUILD}/lpt-mres.dobj
CR_MRES_OBJ = ${BUILD}/cr-mres.dobj
CP_MRES_OBJ = ${BUILD}/cp-mres.dobj
DCS_MRES_OBJ = ${BUILD}/dcs-mres.dobj
GE_MRES_OBJ = ${BUILD}/ge-mres.dobj
DPY_MRES_OBJ = ${BUILD}/dpy-mres.dobj
TTY_MRES_OBJ = ${BUILD}/tty-mres.dobj
WCNSLS_MRES_OBJ = ${BUILD}/wcnsls-mres.dobj
OCNSLS_MRES_OBJ = ${BUILD}/ocnsls-mres.dobj
DSK_MRES_OBJ = ${BUILD}/dsk-mres.dobj
DRM_MRES_OBJ = ${BUILD}/drm-mres.dobj
TAPE_MRES_OBJ = ${BUILD}/tape-mres.dobj
SLV_MRES_OBJ = ${BUILD}/slv-mres.dobj
MEMFS_MRES_OBJ = ${BUILD}/memfs-mres.dobj
DTFS_MRES_OBJ = ${BUILD}/dtfs-mres.dobj
DTFS_MRES_FOLD_PLAN = ${BUILD}/dtfs-mres.fold
BLOCKSET_MRES_OBJ = ${BUILD}/blockset-mres.dobj
BADMAP_MRES_OBJ = ${BUILD}/badmap-mres.dobj
LOGSTORE_MRES_OBJ = ${BUILD}/logstore-mres.dobj
D6FS_MRES_OBJ = ${BUILD}/d6fs-mres.dobj
MRES_PACKAGE_OBJS_LOWMEM = ${CTY_MRES_OBJ} ${CLK_MRES_OBJ} ${DCS_MRES_OBJ} \
	${GE_MRES_OBJ} ${TTY_MRES_OBJ} ${DSK_MRES_OBJ} ${BLOCKSET_MRES_OBJ} \
	${D6FS_MRES_OBJ}
MRES_PACKAGE_OBJS_FULL = ${CTY_MRES_OBJ} ${CLK_MRES_OBJ} ${PTR_MRES_OBJ} \
	${PTP_MRES_OBJ} ${LPT_MRES_OBJ} ${CR_MRES_OBJ} ${CP_MRES_OBJ} \
	${DCS_MRES_OBJ} ${GE_MRES_OBJ} ${DPY_MRES_OBJ} ${TTY_MRES_OBJ} \
	${WCNSLS_MRES_OBJ} ${OCNSLS_MRES_OBJ} ${TAPE_MRES_OBJ} ${DSK_MRES_OBJ} ${DRM_MRES_OBJ} \
	${BLOCKSET_MRES_OBJ} ${LOGSTORE_MRES_OBJ} ${BADMAP_MRES_OBJ} ${MEMFS_MRES_OBJ} ${DTFS_MRES_OBJ} ${SLV_MRES_OBJ} \
	${D6FS_MRES_OBJ}
MRES_PACKAGE_OBJS != if test "${BOOT_PROFILE}" = lowmem; then echo '${MRES_PACKAGE_OBJS_LOWMEM}'; else echo '${MRES_PACKAGE_OBJS_FULL}'; fi
KINIT_FULL != if test "${BOOT_PROFILE}" = lowmem; then echo 0; else echo 1; fi

MINIT_TABLE_ASM = ${BUILD}/module-table-runtime.s
MINIT_TABLE_OBJ = ${BUILD}/module-table-runtime.dobj
END_OBJ = ${BUILD}/image-end.dobj
IMAGE_DXR = ${BUILD}/kinit.dxr
IMAGE_MAP = ${BUILD}/kinit.map
IMAGE_RIM = ${BUILD}/kinit.rim
IMAGE_RAW = ${BUILD}/kinit.raw.words
IMAGE_WORDS = ${BUILD}/kinit.words
IMAGE_LOWMEM_WORDS = ${BUILD}/kinit.lowmem.words
IMAGE_COMPRESSED_WORDS = ${BUILD}/kinit.d6lz.words
IMAGE_BOOT_WORDS != if test "${BOOT_PROFILE}" = lowmem; then echo '${IMAGE_LOWMEM_WORDS}'; else echo '${IMAGE_COMPRESSED_WORDS}'; fi

USERLAND_BUILD_ROOT ?= ${BUILD}/userland-build
INIT_DXR = ${USERLAND_BUILD_ROOT}/userland/init.dxr
MOUNTALL_DXR = ${USERLAND_BUILD_ROOT}/userland/mountall.dxr
LOGIN_DXR = ${USERLAND_BUILD_ROOT}/userland/login.dxr
DSH_DXR = ${USERLAND_BUILD_ROOT}/userland/dsh.dxr
DSHCOMP_DXR = ${USERLAND_BUILD_ROOT}/userland/dshcomp.dxr
XARGS_DXR = ${USERLAND_BUILD_ROOT}/userland/xargs.dxr
SED_DXR = ${USERLAND_BUILD_ROOT}/userland/sed.dxr
CONFIGCTL_DXR = ${USERLAND_BUILD_ROOT}/userland/configctl.dxr
MAKE_DXR = ${USERLAND_BUILD_ROOT}/userland/make.dxr
SYSTEM_COMMAND_NAMES = ECHO CAT LS MKDIR RM PWD STAT TOUCH DATE RMDIR CP CHMOD \
	CHOWN MKFS.DTFS FSCK.DTFS MOUNT MOUNT.DTFS UNMOUNT MV HEXDUMP PS DEVS \
	MODS MOUNTS FREE MEMSTAT SYSCTL DF TTYOUT HALT CLEAR DPYVIEW KILL
SYSTEM_UTILITY_MISC_NAMES = TRUE FALSE CHECK BASENAME DIRNAME ENV WHICH EXPR BC \
	TEST BRACKET CAL
SYSTEM_UTILITY_TEXT_NAMES = CMP HEAD GREP WC TEE TAIL CUT SORT DD STRINGS
SYSTEM_UTILITY_DOC_NAMES = MAN HELP APROPOS TREE FIND DU
SYSTEM_LIBRARY_COMMAND_NAMES = ${SYSTEM_COMMAND_NAMES} ${SYSTEM_UTILITY_MISC_NAMES} \
	${SYSTEM_UTILITY_TEXT_NAMES} ${SYSTEM_UTILITY_DOC_NAMES} LOGD LOGDUMP
SYSTEM_LIBRARY_COMMAND_DXRS != for n in ${SYSTEM_LIBRARY_COMMAND_NAMES}; do printf '%s ' "${USERLAND_BUILD_ROOT}/userland/program/$$n.dxr"; done
SYSTEM_LIBRARY_COMMAND_D6LZ_DXRS != for n in ${SYSTEM_LIBRARY_COMMAND_NAMES}; do printf '%s ' "${USERLAND_BUILD_ROOT}/userland/program/$$n.d6lz.dxr"; done
SYSTEM_LIBRARY_COMMAND_ARGS != for n in ${SYSTEM_LIBRARY_COMMAND_NAMES}; do printf '%s ' "-f /SYSTEM/EXEC/$$n:${USERLAND_BUILD_ROOT}/userland/program/$$n.d6lz.dxr:555:dxr"; done; printf '%s' "-f /SYSTEM/EXEC/[:${USERLAND_BUILD_ROOT}/userland/program/BRACKET.d6lz.dxr:555:dxr"
XARGS_D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/xargs.d6lz.dxr
SED_D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/sed.d6lz.dxr
CONFIGCTL_D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/configctl.d6lz.dxr
MAKE_D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/make.d6lz.dxr
LOGIN_D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/login.d6lz.dxr
DSH_D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/dsh.d6lz.dxr
TSFSPROBE_DXR = ${USERLAND_BUILD_ROOT}/userland/tsfsprobe.dxr
MOUNTTSFS_DXR = ${USERLAND_BUILD_ROOT}/userland/mounttsfs.dxr
DTFSPROBE_DXR = ${USERLAND_BUILD_ROOT}/userland/dtfsprobe.dxr
D6LZ_DXR = ${USERLAND_BUILD_ROOT}/userland/d6lz.dxr
LOGDRAIN_DXR = ${USERLAND_BUILD_ROOT}/userland/logdrain.dxr
SYSTEM_INIT_DXR ?= ${INIT_DXR}
SYSTEM_MOUNTALL_DXR ?= ${MOUNTALL_DXR}
SYSTEM_LOGIN_DXR ?= ${LOGIN_D6LZ_DXR}
SYSTEM_DSH_DXR ?= ${DSH_D6LZ_DXR}
SYSTEM_XARGS_DXR ?= ${XARGS_D6LZ_DXR}
SYSTEM_SED_DXR ?= ${SED_D6LZ_DXR}
SYSTEM_CONFIGCTL_DXR ?= ${CONFIGCTL_D6LZ_DXR}
SYSTEM_MAKE_DXR ?= ${MAKE_D6LZ_DXR}
SYSTEM_TSFSPROBE_DXR ?= ${TSFSPROBE_DXR}
SYSTEM_MOUNTTSFS_DXR ?= ${MOUNTTSFS_DXR}
SYSTEM_DTFSPROBE_DXR ?= ${DTFSPROBE_DXR}
SYSTEM_D6LZ_DXR ?= ${D6LZ_DXR}
SYSTEM_LOGDRAIN_DXR ?= ${LOGDRAIN_DXR}
DAS_NATIVE_BUILD ?= ${BUILD}/external/das
DAS_NATIVE_DRIVER = ${DAS_NATIVE_BUILD}/das.dxr
DAS_NATIVE_PHASE1 = ${DAS_NATIVE_BUILD}/das1.dxr
DAS_NATIVE_PHASE2 = ${DAS_NATIVE_BUILD}/das2.dxr
DAS_MANUAL = ${DAS_REPO}/DAS.SIXMD
DAIMOS_TOOLS_NATIVE_BUILD ?= ${BUILD}/external/daimos-tools
DAIMOS_NATIVE_DARC = ${DAIMOS_TOOLS_NATIVE_BUILD}/darc.dxr
DAIMOS_NATIVE_DLINK = ${DAIMOS_TOOLS_NATIVE_BUILD}/dlink.dxr
DAIMOS_NATIVE_OBJDUMP = ${DAIMOS_TOOLS_NATIVE_BUILD}/objdump.dxr
DAIMOS_TOOLS_MANUALS = ${DAIMOS_TOOLS_REPO}/DARC.SIXMD \
	${DAIMOS_TOOLS_REPO}/DLINK.SIXMD ${DAIMOS_TOOLS_REPO}/OBJDUMP.SIXMD
KCC_NATIVE_BUILD ?= ${BUILD}/external/kcc
KCC_NATIVE_DRIVER = ${KCC_NATIVE_BUILD}/KCC.dxr
KCC_NATIVE_KCPP = ${KCC_NATIVE_BUILD}/KCPP.dxr
KCC_NATIVE_KPARSE = ${KCC_NATIVE_BUILD}/KPARSE.dxr
KCC_NATIVE_KGEN = ${KCC_NATIVE_BUILD}/KGEN.dxr
KCC_NATIVE_KOPT = ${KCC_NATIVE_BUILD}/KOPT.dxr
DAS_NATIVE_STAMP = ${DAS_NATIVE_BUILD}/.built
DAIMOS_TOOLS_NATIVE_STAMP = ${DAIMOS_TOOLS_NATIVE_BUILD}/.built
KCC_NATIVE_STAMP = ${KCC_NATIVE_BUILD}/.built
USERLAND_STAMP = ${USERLAND_BUILD_ROOT}/userland/.boot-built
USERLAND_D6LZ_STAMP = ${USERLAND_BUILD_ROOT}/userland/.d6lz-built
DAIMOS_TOOLS_NATIVE_INPUTS != find ${DAIMOS_TOOLS_REPO}/native -maxdepth 1 -type f \( -name '*.c' -o -name '*.h' \) -print 2>/dev/null
KCC_NATIVE_INPUTS != find ${KCC_REPO} -maxdepth 1 -type f \( -name '*.c' -o -name '*.h' \) -print 2>/dev/null; find ${KCC_REPO}/runtime ${KCC_REPO}/self/include -maxdepth 1 -type f \( -name '*.c' -o -name '*.s' -o -name '*.h' \) -print 2>/dev/null
SYSTEM_PASSWD_TEXT ?= ${PROJECT_ROOT}/userland/config/passwd
SYSTEM_SYSTEM_TEXT ?= ${PROJECT_ROOT}/userland/config/system
SYSTEM_INITTAB_TEXT_DEFAULT != if test "${BOOT_PROFILE}" = lowmem; then echo '${PROJECT_ROOT}/userland/config/inittab.lowmem'; elif test "${DPY_LOGIN}" = 1; then echo '${PROJECT_ROOT}/userland/config/inittab.dpy'; else echo '${PROJECT_ROOT}/userland/config/inittab'; fi
SYSTEM_FSTAB_TEXT_DEFAULT != if test "${BOOT_PROFILE}" = lowmem; then echo '${PROJECT_ROOT}/userland/config/fstab.lowmem'; else echo '${PROJECT_ROOT}/userland/config/fstab'; fi
SYSTEM_INITTAB_TEXT ?= ${SYSTEM_INITTAB_TEXT_DEFAULT}
SYSTEM_FSTAB_TEXT ?= ${SYSTEM_FSTAB_TEXT_DEFAULT}
SYSTEM_CONFIG_BUILD = ${BUILD}/config
SYSTEM_INITTAB_S6REC = ${SYSTEM_CONFIG_BUILD}/INITTAB.s6rec
SYSTEM_PASSWD_S6REC = ${SYSTEM_CONFIG_BUILD}/PASSWD.s6rec
SYSTEM_FSTAB_S6REC = ${SYSTEM_CONFIG_BUILD}/FSTAB.s6rec
SYSTEM_SYSTEM_S6REC = ${SYSTEM_CONFIG_BUILD}/SYSTEM.s6rec
SYSTEM_CONFIG_S6REC = ${SYSTEM_INITTAB_S6REC} ${SYSTEM_PASSWD_S6REC} \
	${SYSTEM_FSTAB_S6REC} ${SYSTEM_SYSTEM_S6REC}
# Optional additional mkd6fs arguments are primarily useful to target tests
# that need to place extra executables/data in an otherwise standard root.
# Keep the default empty so production images are unchanged.
D6FS_EXTRA_ARGS ?=
D6FS_DISK_EXTRA_ARGS ?=

# Host-side picture staging.  img2dpic emits DAIMOS binary-word files, so the
# media builders can import them directly without target-side conversion.
# Host basenames are folded to uppercase for the SIXBIT filesystem namespace.
# Keep names shell-safe because mkd6fs receives file specifications on argv.
PICTURE_FILES != set -- '${PICTURE_DIR}'/*; for src do \
	if test -f "$$src"; then printf '%s\n' "$$src"; fi; \
	done
PICTURE_NAME_ERRORS != for src in ${PICTURE_FILES}; do \
	leaf=$$(basename "$$src" | tr '[:lower:]' '[:upper:]'); \
	case "$$leaf" in ''|*[!A-Z0-9._+-]*) printf '%s ' "$$src" ;; esac; \
	n=$$(printf '%s' "$$leaf" | wc -c); \
	if test "$$n" -gt 24; then printf '%s ' "$$src"; fi; \
	done
PICTURE_D6FS_ARGS != for src in ${PICTURE_FILES}; do \
	leaf=$$(basename "$$src" | tr '[:lower:]' '[:upper:]'); \
	printf '%s ' "-f /PICTURES/$$leaf:$$src:444:binwords"; \
	done
MANUAL_BUILD = ${BUILD}/manual
SYSTEM_MANUAL_SRCS != set -- ${PROJECT_ROOT}/userland/manual/*.SIXMD ${DAS_MANUAL} ${DAIMOS_TOOLS_MANUALS}; for f do test -f "$$f" && printf '%s ' "$$f"; done
SYSTEM_MANUAL_PAGES = ${MANUAL_BUILD}/PAGES.s6rec
SYSTEM_MANUAL_INDEX = ${MANUAL_BUILD}/INDEX.s6rec
SYSTEM_COMMAND_INDEX = ${MANUAL_BUILD}/COMMANDS.s6rec
SYSTEM_MANUALS = ${SYSTEM_MANUAL_PAGES} ${SYSTEM_MANUAL_INDEX} \
	${SYSTEM_COMMAND_INDEX}
SYSTEM_MANUAL_ARGS = -f "/SYSTEM/MANUAL/PAGES:${SYSTEM_MANUAL_PAGES}:444:binwords" \
	-f "/SYSTEM/MANUAL/INDEX:${SYSTEM_MANUAL_INDEX}:444:binwords" \
	-f "/SYSTEM/MANUAL/COMMANDS:${SYSTEM_COMMAND_INDEX}:444:binwords"
ASMUTILS_SOURCE_DIR = ${PROJECT_ROOT}/userland/optional/asmutils
ASMUTILS_SOURCE_BUILD = ${BUILD}/asmutils-source
ASMUTILS_ASM_NAMES != for f in ${ASMUTILS_SOURCE_DIR}/*.S; do test -f "$$f" && basename "$$f"; done
ASMUTILS_ASM_S6REC != for n in ${ASMUTILS_ASM_NAMES}; do printf '%s ' "${ASMUTILS_SOURCE_BUILD}/$$n.s6rec"; done
ASMUTILS_MAKEFILE_S6REC = ${ASMUTILS_SOURCE_BUILD}/MAKEFILE.s6rec
ASMUTILS_SOURCE_S6REC = ${ASMUTILS_ASM_S6REC} ${ASMUTILS_MAKEFILE_S6REC}
ASMUTILS_SOURCE_ARGS != for n in ${ASMUTILS_ASM_NAMES}; do printf '%s ' "-f /OPTION/BASE/SOURCE/ASMUTILS/$$n:${ASMUTILS_SOURCE_BUILD}/$$n.s6rec:444:binwords"; done; printf '%s' "-f /OPTION/BASE/SOURCE/ASMUTILS/MAKEFILE:${ASMUTILS_MAKEFILE_S6REC}:444:binwords"
USERLAND_INPUTS = ${PROJECT_ROOT}/userland/libc/Makefile \
	${PROJECT_ROOT}/userland/libc/crt0.s \
	${PROJECT_ROOT}/userland/libc/dsys.h \
	${PROJECT_ROOT}/userland/libc/memcpy.s \
	${PROJECT_ROOT}/userland/libc/memmove.s \
	${PROJECT_ROOT}/userland/libc/syscall.s \
	${PROJECT_ROOT}/userland/libc/syscall_helpers.s \
	${PROJECT_ROOT}/userland/libc/u.c \
	${PROJECT_ROOT}/userland/libc/u.h \
	${PROJECT_ROOT}/userland/libc/text.c \
	${PROJECT_ROOT}/userland/libc/text.h \
	${PROJECT_ROOT}/userland/logstore/logdrain.c \
	${PROJECT_ROOT}/userland/exec/COMMANDS \
	${PROJECT_ROOT}/userland/exec/cmd.h \
	${PROJECT_ROOT}/userland/exec/commands.c \
	${PROJECT_ROOT}/userland/exec/standalone.c \
	${PROJECT_ROOT}/userland/exec/utility.c \
	${PROJECT_ROOT}/userland/exec/utility.h \
	${PROJECT_ROOT}/userland/exec/utility_standalone.c \
	${PROJECT_ROOT}/userland/exec/xargs.c \
	${PROJECT_ROOT}/userland/exec/sed.c \
	${PROJECT_ROOT}/userland/exec/configctl.c \
	${PROJECT_ROOT}/userland/exec/make.c \
	${PROJECT_ROOT}/userland/logstore/logcompat.c \
	${PROJECT_ROOT}/userland/dsh/dsh.c \
	${PROJECT_ROOT}/userland/dsh/dshcomp.c \
	${PROJECT_ROOT}/userland/dsh/dsh.h \
	${PROJECT_ROOT}/userland/dsh/dsh_env.c \
	${PROJECT_ROOT}/userland/dsh/dsh_exec.c \
	${PROJECT_ROOT}/userland/dsh/dsh_lex.c \
	${PROJECT_ROOT}/userland/dsh/dsh_lex.h \
	${PROJECT_ROOT}/userland/dsh/dsh_parse.c \
	${PROJECT_ROOT}/userland/dsh/dsh_parse.h \
	${PROJECT_ROOT}/userland/dsh/dsh_s6.c \
	${PROJECT_ROOT}/userland/dsh/dsh_var_pdp6.s \
	${PROJECT_ROOT}/userland/d6lz/d6lz.c \
	${PROJECT_ROOT}/userland/init/init.c \
	${PROJECT_ROOT}/userland/login/login.c \
	${PROJECT_ROOT}/userland/tsfs/tsfs_scan.h \
	${PROJECT_ROOT}/userland/tsfs/tsfs_scan.c \
	${PROJECT_ROOT}/userland/tsfs/tsfsprobe.c \
	${PROJECT_ROOT}/userland/tsfs/mount_tsfs.c \
	${PROJECT_ROOT}/userland/dtfs/dtfs_probe.h \
	${PROJECT_ROOT}/userland/dtfs/dtfs_probe.c \
	${PROJECT_ROOT}/userland/dtfs/dtfsprobe.c \
	${PROJECT_ROOT}/userland/config/inittab \
	${PROJECT_ROOT}/userland/config/passwd \
	${PROJECT_ROOT}/userland/config/system \
	${PROJECT_ROOT}/userland/Makefile \
	${PROJECT_ROOT}/system/kernel/proc/syscall.h \
	${PROJECT_ROOT}/system/kernel/fs/dtfs_media.h

STAGE1_DEVICE != if test "${BOOT}" = auto; then if test "${ROOT}" = drum; then echo drm; else echo dsk; fi; else echo "${BOOT}"; fi
STAGE1_SRC = ${PROJECT_ROOT}/system/stand/pdp6/${STAGE1_DEVICE}/stage1.s
STAGE1_COMMON = ${PROJECT_ROOT}/system/stand/pdp6/common/decompressor.inc \
	${PROJECT_ROOT}/system/stand/pdp6/common/stage1-error.inc \
	${PROJECT_ROOT}/system/stand/pdp6/common/sixbit-fixed.inc
STAGE1_DXR = ${BUILD}/stage1-${STAGE1_DEVICE}.dxr
STAGE1_TAPE = ${BUILD}/stage1-${STAGE1_DEVICE}.pt
DISK_DIR = ${BUILD}/disk
DISK0_IMAGE = ${DISK_DIR}/dsk0.dsk
DISK1_IMAGE = ${DISK_DIR}/dsk1.dsk
DISK2_IMAGE = ${DISK_DIR}/dsk2.dsk
DISK3_IMAGE = ${DISK_DIR}/dsk3.dsk
DISK_IMAGES = ${DISK0_IMAGE} ${DISK1_IMAGE} ${DISK2_IMAGE} ${DISK3_IMAGE}
DISK_IMAGE = ${DISK0_IMAGE}
DISK_STAMP = ${DISK_DIR}/.built
SIMH_INI = ${BUILD}/boot.ini
SIMH_LOG = ${BUILD}/boot.simh.log
MEDIA_DIR = ${BUILD}/media
DTC0_IMAGE = ${MEDIA_DIR}/dtc0.tap
DTC1_IMAGE = ${MEDIA_DIR}/dtc1.tap
DTC2_IMAGE = ${MEDIA_DIR}/dtc2.tap
TSFS_ROOT_MANIFEST = ${BUILD}/tsfs-root.manifest
TSFS_ROOT_PREFIX = ${MEDIA_DIR}/tsfs-root
TSFS_ROOT_IMAGE = ${TSFS_ROOT_PREFIX}0.dta
MTC0_IMAGE = ${MEDIA_DIR}/mtc0.tap
MTC1_IMAGE = ${MEDIA_DIR}/mtc1.tap
DRUM_DIR = ${BUILD}/drum
DR0_IMAGE = ${DRUM_DIR}/dsk0.dsk
DR1_IMAGE = ${DRUM_DIR}/dsk1.dsk
DR2_IMAGE = ${DRUM_DIR}/dsk2.dsk
DR3_IMAGE = ${DRUM_DIR}/dsk3.dsk
DRUM_IMAGES = ${DR0_IMAGE} ${DR1_IMAGE} ${DR2_IMAGE} ${DR3_IMAGE}
DRUM_STAMP = ${DRUM_DIR}/.built

ROOT_MEDIA_DEPS != if test "${ROOT}" = tape; then echo '${TSFS_ROOT_IMAGE}'; fi
PTP_OUTPUT = ${MEDIA_DIR}/ptp.out
CP_OUTPUT = ${MEDIA_DIR}/cp.out
LPT_OUTPUT = ${MEDIA_DIR}/lpt.out
BOOT_MEDIA_STAMP = ${BUILD}/.boot-media-${STAGE1_DEVICE}
BOOT_COMMANDS = ${BUILD}/boot-commands-${STAGE1_DEVICE}.simh
PTR_BOOT1_WORDS = ${BUILD}/ptr-boot1.words
PTR_BOOT2_WORDS = ${BUILD}/ptr-boot2.words
PTR_BOOT1 = ${MEDIA_DIR}/ptr-boot1.pt
PTR_BOOT2 = ${MEDIA_DIR}/ptr-boot2.pt
SIMH_DCS0_PORT ?= 1101
SIMH_GE0_PORT ?= 1201
SIMH_PCLK_MODE ?= REALTIME
SIMH_DPY_MODE ?= HEADLESS

.PHONY: image boot permanent-size clean force-simh-config

force-simh-config:

check-config:
	@case '${BOOT_PROFILE}' in full|lowmem) ;; *) echo "unsupported BOOT_PROFILE=${BOOT_PROFILE}; expected full or lowmem" >&2; exit 2 ;; esac
	@case '${STAGE1_DEVICE}' in dsk|drm|dtc|mtc|ptr) ;; *) echo "unsupported BOOT=${BOOT}; expected auto, dsk, drm, dtc, mtc, or ptr" >&2; exit 2 ;; esac
	@case '${DPY_LOGIN}' in 0|1) ;; *) echo "DPY_LOGIN must be 0 or 1" >&2; exit 2 ;; esac
	@test -z '${PICTURE_NAME_ERRORS}' || { echo 'picture filenames must map to <=24 characters from A-Z, 0-9, dot, underscore, plus, or minus: ${PICTURE_NAME_ERRORS}' >&2; exit 2; }

${BUILD}:
	mkdir -p $@

${MEDIA_DIR}:
	mkdir -p $@

${KINIT_STAMP}: ${KERNEL_INPUTS} ${PDP10_KCC} ${BUILD}
	${MAKE} -C ${KERNEL_DIR} kinit BUILD='${KINIT_BUILD}' PDP10_PREFIX='${PDP10_PREFIX}' KINIT_STACK_RESERVE_WORDS='${KINIT_STACK_RESERVE_WORDS}' KINIT_STACK_WATERMARK='${KINIT_STACK_WATERMARK}' KINIT_BADMAP='${KINIT_BADMAP}' KINIT_FULL='${KINIT_FULL}' PROC_STACK_WATERMARK='${PROC_STACK_WATERMARK}' VM_BACKEND='${VM_BACKEND}'
	touch $@

${KCORE_STAMP}: ${KERNEL_INPUTS} ${PDP10_KCC} ${BUILD}
	${MAKE} -C ${KERNEL_DIR} kcore BUILD='${KCORE_BUILD}' PDP10_PREFIX='${PDP10_PREFIX}' KINIT_STACK_WATERMARK='${KINIT_STACK_WATERMARK}' PROC_BOOT_USERS='${PROC_BOOT_USERS}' PROC_STACK_WATERMARK='${PROC_STACK_WATERMARK}' PROC_SWAP_VERIFY_PURE='${PROC_SWAP_VERIFY_PURE}' VM_BACKEND='${VM_BACKEND}'
	touch $@

${KINIT_ASM} ${KINIT_LATE_ASM}: ${KINIT_STAMP}
	@test -f $@

${KCORE_FIXED_ASM} ${MRES_ASM}: ${KCORE_STAMP}
	@test -f $@

KERNEL_ASM_OBJS = ${BUILD}/kinit.dobj ${BUILD}/kinit_diag.dobj ${BUILD}/module_minit.dobj ${BUILD}/kinit_boot.dobj ${BUILD}/root_select.dobj ${BUILD}/tsfs_boot.dobj ${BUILD}/d6fs_boot.dobj ${BUILD}/d6fs_super_boot.dobj ${BUILD}/blockset_boot.dobj ${BUILD}/auxstore_boot.dobj ${BUILD}/logstore_boot.dobj ${BUILD}/logstore.dobj ${BUILD}/proc_boot.dobj ${BUILD}/proc_swap_boot.dobj ${BUILD}/kinit_io_pdp6.dobj ${BUILD}/module_io.dobj ${BUILD}/bootbanner_generated.dobj ${BUILD}/kinit_late_begin_pdp6.dobj ${BUILD}/proc_late.dobj ${BUILD}/mm_boot.dobj ${BUILD}/kinit_late.dobj ${BUILD}/kinit_late_end.dobj ${BUILD}/d6lz_fixed.dobj ${BUILD}/exec.dobj ${BUILD}/d6lz.dobj ${BUILD}/mm.dobj ${BUILD}/proc.dobj ${BUILD}/backstore.dobj ${BUILD}/vm_backend.dobj ${BUILD}/proc_swap.dobj ${BUILD}/file.dobj ${BUILD}/ret.dobj ${BUILD}/dsk270.dobj ${BUILD}/drm236.dobj ${BUILD}/kfmt_pdp6.dobj ${BUILD}/s6rec.dobj ${BUILD}/vfs.dobj ${BUILD}/fs_mres.dobj ${BUILD}/file_runtime.dobj ${BUILD}/pipe.dobj ${BUILD}/monitorfs_runtime.dobj ${BUILD}/proc_pdp6.dobj ${BUILD}/proc_run.dobj ${BUILD}/device_state.dobj ${BUILD}/syscall_info.dobj ${BUILD}/pclk.dobj ${BUILD}/syscall_dispatch_pdp6.dobj ${BUILD}/mach_user_pdp6.dobj ${BUILD}/vm_backend_pdp6.dobj ${BUILD}/kcore_pi_pdp6.dobj ${BUILD}/bcache_pdp6.dobj ${BUILD}/storage_router.dobj ${BUILD}/kcore_end.dobj ${BUILD}/cty_io.dobj ${BUILD}/clk_io.dobj ${BUILD}/ptr_io.dobj ${BUILD}/ptp_io.dobj ${BUILD}/lpt_io.dobj ${BUILD}/cr_io.dobj ${BUILD}/cp_io.dobj ${BUILD}/dcs_io.dobj ${BUILD}/ge_io.dobj ${BUILD}/dpy_io.dobj ${BUILD}/dpy_text.dobj ${BUILD}/tty_io.dobj ${BUILD}/wcnsls_io.dobj ${BUILD}/ocnsls_io.dobj ${BUILD}/dsk_io.dobj ${BUILD}/drm236_io.dobj ${BUILD}/tape_io.dobj ${BUILD}/slv_io.dobj ${BUILD}/memfs_data.dobj ${BUILD}/memfs_snapshot.dobj ${BUILD}/memfs_runtime.dobj ${BUILD}/dtfs.dobj ${BUILD}/dtfs_runtime.dobj ${BUILD}/tsfs.dobj ${BUILD}/tsfs_runtime.dobj ${BUILD}/blockset_dispatch.dobj ${BUILD}/badmap_dispatch.dobj ${BUILD}/fs_backing.dobj ${BUILD}/logstore_runtime.dobj ${BUILD}/d6fs_provider.dobj ${BUILD}/d6fs_validate.dobj ${BUILD}/d6fs_runtime.dobj

${KERNEL_ASM_OBJS}: ${KINIT_ASM} ${KINIT_LATE_ASM} ${KCORE_FIXED_ASM} ${MRES_ASM}
	@mkdir -p ${BUILD}
	@base=`basename "$@" .dobj`; src=; if test -f "${KINIT_BUILD}/$$base.s"; then src="${KINIT_BUILD}/$$base.s"; elif test -f "${KCORE_BUILD}/$$base.s"; then src="${KCORE_BUILD}/$$base.s"; fi; test -n "$$src"; ${DAS} ${DASFLAGS} -C -O "$@" "$$src"

force-minit-config:

${MINIT_TABLE_ASM}: ${KERNEL_DIR}/modules/module_table.s force-minit-config
	mkdir -p ${BUILD}
	{ echo ".set KINIT_FULL,${KINIT_FULL}"; echo ".set KINIT_BADMAP,${KINIT_BADMAP}"; cat $<; } > $@

${MINIT_TABLE_OBJ}: ${MINIT_TABLE_ASM} ${BUILD}
	${DAS} ${DASFLAGS} -C -O $@ $<

${END_OBJ}: image/image-end.s ${BUILD}
	${DAS} ${DASFLAGS} -C -O $@ $<

${KCORE_FOLD_PLAN}: ${KCORE_FOLD_OBJS} ${P10FOLD}
	${P10FOLD} -m 4 -P $@ ${KCORE_FOLD_OBJS} >/dev/null

${KCORE_DXR}: ${KCORE_FIXED_OBJS} ${KCORE_FOLD_PLAN}
	libgcc=`${PDP10_GCC} -print-libgcc-file-name`; \
	${DLINK} -b 060 --fold-plan ${KCORE_FOLD_PLAN} -o $@ -M ${KCORE_MAP} \
	    ${KCORE_FIXED_OBJS} $$libgcc

${KCORE_MAP}: ${KCORE_DXR}
	@test -f $@

${KCORE_RIM}: ${KCORE_DXR}
	${DXRCONVERT} --simh -b 060 $< $@

${KCORE_STREAM}: ${KCORE_RIM} ${KCORE_MAP}
	@end=; \
	while read name value rest; do \
		if test "$$name" = "__kcore_low_init_end"; then \
			end=$$value; \
			break; \
		fi; \
	done < ${KCORE_MAP}; \
	test -n "$$end"; words=$$((0$$end - 060)); words=`printf '%o' "$$words"`; \
	${MKSTREAM} -i ${KCORE_RIM} -o $@ -b 060 -w "$$words"

${CTY_MRES_OBJ}: ${BUILD}/cty_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/cty-mres.dxr -M ${BUILD}/cty-mres.map \
	    -R $@ -N cty_mres_package -X cty_pi_handler -X cty_putchar \
	    -X cty_getchar -X cty_pi_tail ${BUILD}/cty_io.dobj

${CLK_MRES_OBJ}: ${BUILD}/clk_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/clk-mres.dxr -M ${BUILD}/clk-mres.map \
	    -R $@ -N clk_mres_package -X clk_pi_handler -X clk_ticks -X clk_pi_service \
	    -X clk_pi_post_handler -X clk_tick_count -X clk_pi_tail \
	    ${BUILD}/clk_io.dobj

${PTR_MRES_OBJ}: ${BUILD}/ptr_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/ptr-mres.dxr -M ${BUILD}/ptr-mres.map \
	    -R $@ -N ptr_mres_package -X ptr_read_words \
	    ${BUILD}/ptr_io.dobj

${PTP_MRES_OBJ}: ${BUILD}/ptp_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/ptp-mres.dxr -M ${BUILD}/ptp-mres.map \
	    -R $@ -N ptp_mres_package -X ptp_write_words \
	    ${BUILD}/ptp_io.dobj

${LPT_MRES_OBJ}: ${BUILD}/lpt_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/lpt-mres.dxr -M ${BUILD}/lpt-mres.map \
	    -R $@ -N lpt_mres_package -X lpt_putchar -X lpt_write_s6rec \
	    ${BUILD}/lpt_io.dobj

${CR_MRES_OBJ}: ${BUILD}/cr_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/cr-mres.dxr -M ${BUILD}/cr-mres.map \
	    -R $@ -N cr_mres_package -X cr_read_words \
	    ${BUILD}/cr_io.dobj

${CP_MRES_OBJ}: ${BUILD}/cp_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/cp-mres.dxr -M ${BUILD}/cp-mres.map \
	    -R $@ -N cp_mres_package -X cp_write_words \
	    ${BUILD}/cp_io.dobj

${DCS_MRES_OBJ}: ${BUILD}/dcs_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/dcs-mres.dxr -M ${BUILD}/dcs-mres.map \
	    -R $@ -N dcs_mres_package -X dcs_pi_handler -X dcs_getchar -X dcs_putchar -X dcs_pi_tail \
	    ${BUILD}/dcs_io.dobj

${GE_MRES_OBJ}: ${BUILD}/ge_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/ge-mres.dxr -M ${BUILD}/ge-mres.map \
	    -R $@ -N ge_mres_package -X ge_pi_handler -X ge_getchar -X ge_putchar \
	    ${BUILD}/ge_io.dobj

${DPY_MRES_OBJ}: ${BUILD}/dpy_io.dobj ${BUILD}/dpy_text.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/dpy-mres.dxr -M ${BUILD}/dpy-mres.map \
	    -R $@ -N dpy_mres_package -X dpy_pi_handler -X dpy_clock_handler \
	    -X dpy_clk_tick_load -X dpy_putchar \
	    -X dpy_banner_init -X dpy_write_words -X dpy_refresh_iowd \
	    ${BUILD}/dpy_io.dobj ${BUILD}/dpy_text.dobj

${TTY_MRES_OBJ}: ${BUILD}/tty_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/tty-mres.dxr -M ${BUILD}/tty-mres.map \
	    -R $@ -N tty_mres_package -X tty_putchar -X tty_getchar -X tty_cty_putchar_address \
	    -X tty_dcs_putchar_address -X tty_ge_putchar_address \
	    -X tty_cty_getchar_address -X tty_dcs_getchar_address \
	    -X tty_ge_getchar_address -X tty_write_s6rec -X tty_read_s6rec \
	    -X tty_dpy_putchar_address -X tty_ttydpy_putchar -X tty_ttydpy_getchar \
	    ${BUILD}/tty_io.dobj

${WCNSLS_MRES_OBJ}: ${BUILD}/wcnsls_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/wcnsls-mres.dxr -M ${BUILD}/wcnsls-mres.map \
	    -R $@ -N wcnsls_mres_package -X wcnsls_read_words -X wcnsls_write_words \
	    ${BUILD}/wcnsls_io.dobj

${OCNSLS_MRES_OBJ}: ${BUILD}/ocnsls_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/ocnsls-mres.dxr -M ${BUILD}/ocnsls-mres.map \
	    -R $@ -N ocnsls_mres_package -X ocnsls_read ${BUILD}/ocnsls_io.dobj

${DSK_MRES_OBJ}: ${BUILD}/dsk_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/dsk-mres.dxr -M ${BUILD}/dsk-mres.map \
	    -R $@ -N dsk_mres_package -X dsk_pi_handler -X dsk_read_sector \
	    -X dsk_write_sector -X dsk_dct_handler -X dsk_watchdog_tick \
	    ${BUILD}/dsk_io.dobj

${DRM_MRES_OBJ}: ${BUILD}/drm236_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/drm-mres.dxr -M ${BUILD}/drm-mres.map \
	    -R $@ -N drm236_mres_package -X drm236_pi_handler \
	    -X drm236_read_block_service -X drm236_write_block_service \
	    ${BUILD}/drm236_io.dobj

${TAPE_MRES_OBJ}: ${BUILD}/tape_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/tape-mres.dxr -M ${BUILD}/tape-mres.map \
	    -R $@ -N tape_mres_package -X tape_pi_handler -X dtc_read_block \
	    -X mtc_service -X dtc_write_block -X tape_dct_handler ${BUILD}/tape_io.dobj

${SLV_MRES_OBJ}: ${BUILD}/slv_io.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/slv-mres.dxr -M ${BUILD}/slv-mres.map \
	    -R $@ -N slv_mres_package -X slv_pi_handler ${BUILD}/slv_io.dobj

${MEMFS_MRES_OBJ}: ${BUILD}/memfs_data.dobj ${BUILD}/memfs_snapshot.dobj ${BUILD}/memfs_runtime.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/memfs-mres.dxr -M ${BUILD}/memfs-mres.map \
	    -R $@ -N memfs_mres_package -X memfs_mres_dispatch -X memfs_mres_usage -X memfs_data_reclaim -X memfs_mres_shutdown \
	    ${BUILD}/memfs_data.dobj ${BUILD}/memfs_snapshot.dobj ${BUILD}/memfs_runtime.dobj

${DTFS_MRES_FOLD_PLAN}: ${BUILD}/dtfs_runtime.dobj ${BUILD}/tsfs_runtime.dobj ${P10FOLD}
	${P10FOLD} -m 4 -P $@ ${BUILD}/dtfs_runtime.dobj ${BUILD}/tsfs_runtime.dobj >/dev/null

${DTFS_MRES_OBJ}: ${BUILD}/dtfs.dobj ${BUILD}/dtfs_runtime.dobj ${BUILD}/tsfs.dobj ${BUILD}/tsfs_runtime.dobj ${KCORE_MAP} ${DTFS_MRES_FOLD_PLAN}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/dtfs-mres.dxr -M ${BUILD}/dtfs-mres.map \
	    --fold-plan ${DTFS_MRES_FOLD_PLAN} \
	    -R $@ -N dtfs_mres_package -X dtfs_mres_dispatch \
	    -X dtfs_dtc_read_jump -X dtfs_dtc_write_jump \
	    -X dtfs_mount_unit -X tsfs_mres_dispatch -X dtfs_dir \
	    ${BUILD}/dtfs.dobj ${BUILD}/dtfs_runtime.dobj ${BUILD}/tsfs.dobj ${BUILD}/tsfs_runtime.dobj

${BLOCKSET_MRES_OBJ}: ${BUILD}/blockset_dispatch.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/blockset-mres.dxr -M ${BUILD}/blockset-mres.map \
	    -R $@ -N blockset_mres_package -X blockset_map_block \
	    -X blockset_boot -X blockset_read_block -X blockset_write_block \
	    -X blockset_backend_read_jump -X blockset_backend_write_jump \
	    ${BUILD}/blockset_dispatch.dobj


${BADMAP_MRES_OBJ}: ${BUILD}/badmap_dispatch.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/badmap-mres.dxr -M ${BUILD}/badmap-mres.map \
	    -R $@ -N badmap_mres_package -X badmap_map_block -X badmap_state \
	    -X badmap_read_block -X badmap_write_block \
	    -X badmap_backend_read_jump -X badmap_backend_write_jump \
	    -X badmap_base_map_jump ${BUILD}/badmap_dispatch.dobj

${LOGSTORE_MRES_OBJ}: ${BUILD}/logstore_runtime.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/logstore-mres.dxr -M ${BUILD}/logstore-mres.map \
	    -R $@ -N logstore_mres_package -X logstore_mres_dispatch \
	    -X logstore_mres_state -X logstore_backend_read_jump \
	    -X logstore_backend_write_jump ${BUILD}/logstore_runtime.dobj

${D6FS_MRES_OBJ}: ${BUILD}/fs_backing.dobj ${BUILD}/d6fs_provider.dobj ${BUILD}/d6fs_validate.dobj ${BUILD}/d6fs_runtime.dobj ${KCORE_MAP}
	${DLINK} -b 0 -A ${KCORE_MAP} -o ${BUILD}/d6fs-mres.dxr -M ${BUILD}/d6fs-mres.map \
	    -R $@ -N d6fs_mres_package -X d6fs_mres_dispatch \
	    -X fs_backing_root_read -X fs_backing_root_write \
	    -X fs_backing_root_read_jump -X fs_backing_root_write_jump \
	    -X fs_backing_direct_read -X fs_backing_direct_write \
	    -X fs_backing_direct_read_jump -X fs_backing_direct_write_jump \
	    -X fs_backing_direct_drm_read_jump -X fs_backing_direct_drm_write_jump \
	    ${BUILD}/fs_backing.dobj ${BUILD}/d6fs_provider.dobj ${BUILD}/d6fs_validate.dobj ${BUILD}/d6fs_runtime.dobj

# KINIT contains one load slot whose size must exactly match initialized KCORE.
# Read the absolute end symbol using shell built-ins only, then emit the tiny
# assembler reservation directly.  This deliberately avoids AWK/sed so the
# step remains suitable for a small future native DAIMOS build environment.
${KCORE_EMBED_ASM}: ${KCORE_MAP} ${BUILD}
	@end=; \
	while read name value rest; do \
		if test "$$name" = "__kcore_low_init_end"; then \
			end=$$value; \
			break; \
		fi; \
	done < ${KCORE_MAP}; \
	test -n "$$end"; \
	words=$$((0$$end - 060)); \
	{ \
		printf '%s\n' \
		    '        .text' \
		    '        .globl __kcore_load_begin' \
		    '        .globl __kcore_load_end' \
		    '__kcore_load_begin:'; \
		printf '        .block %o\n' "$$words"; \
		printf '%s\n' '__kcore_load_end:'; \
	} > $@

${KCORE_EMBED_OBJ}: ${KCORE_EMBED_ASM}
	${DAS} ${DASFLAGS} -C -O $@ $<

${SYSTEM_CONFIG_BUILD}:
	mkdir -p $@

${SYSTEM_INITTAB_S6REC}: ${SYSTEM_INITTAB_TEXT}
	mkdir -p ${SYSTEM_CONFIG_BUILD}
	${S6TEXT} --encode $< $@

${SYSTEM_FSTAB_S6REC}: ${SYSTEM_FSTAB_TEXT}
	mkdir -p ${SYSTEM_CONFIG_BUILD}
	${S6TEXT} --encode $< $@

${SYSTEM_PASSWD_S6REC}: ${SYSTEM_PASSWD_TEXT}
	mkdir -p ${SYSTEM_CONFIG_BUILD}
	${S6TEXT} --encode $< $@

${SYSTEM_SYSTEM_S6REC}: ${SYSTEM_SYSTEM_TEXT}
	mkdir -p ${SYSTEM_CONFIG_BUILD}
	${S6TEXT} --encode $< $@

${ASMUTILS_SOURCE_BUILD}:
	mkdir -p $@

ASMUTILS_ASM_SRCS != for f in ${ASMUTILS_SOURCE_DIR}/*.S; do test -f "$$f" && printf '%s ' "$$f"; done
ASMUTILS_SOURCE_STAMP = ${ASMUTILS_SOURCE_BUILD}/.encoded

${ASMUTILS_SOURCE_STAMP}: ${ASMUTILS_ASM_SRCS} ${ASMUTILS_SOURCE_DIR}/Makefile
	@set -eu; mkdir -p '${ASMUTILS_SOURCE_BUILD}'; \
	for src in ${ASMUTILS_ASM_SRCS}; do \
		n=`basename "$$src"`; ${S6TEXT} --encode "$$src" "${ASMUTILS_SOURCE_BUILD}/$$n.s6rec"; \
	done; \
	tmp='${ASMUTILS_SOURCE_BUILD}/MAKEFILE.native.$$$$'; \
	trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	awk '{ line=$$0; if (substr(line,1,1)=="\t") line="> " substr(line,2); line=toupper(line); print line; }' \
	    ${ASMUTILS_SOURCE_DIR}/Makefile > "$$tmp"; \
	${S6TEXT} --encode "$$tmp" ${ASMUTILS_MAKEFILE_S6REC}; \
	rm -f "$$tmp"; trap - 0 1 2 3 15; touch '$@'

${ASMUTILS_SOURCE_S6REC}: ${ASMUTILS_SOURCE_STAMP}
	@test -f '$@'

${MANUAL_BUILD}:
	mkdir -p $@

${SYSTEM_MANUAL_PAGES}: ${SYSTEM_MANUAL_SRCS}
	@set -eu; mkdir -p ${MANUAL_BUILD}; tmp='$@.txt.$$$$'; trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	: > "$$tmp"; \
	for f in ${SYSTEM_MANUAL_SRCS}; do \
		n=$$(basename "$$f" .SIXMD); \
		${SIXMD_CHECK} "$$f" "$$n"; \
		cat "$$f" >> "$$tmp"; \
	done; \
	${S6FILTER} "$$tmp" > $@; \
	rm -f "$$tmp"; trap - 0 1 2 3 15

${SYSTEM_MANUAL_INDEX}: ${SYSTEM_MANUAL_SRCS}
	@set -eu; mkdir -p ${MANUAL_BUILD}; tmp='$@.txt.$$$$'; trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	: > "$$tmp"; \
	for f in ${SYSTEM_MANUAL_SRCS}; do \
		sed -n 's/^# @?//p' "$$f" | head -n 1 >> "$$tmp"; \
	done; \
	${S6FILTER} "$$tmp" > $@; \
	rm -f "$$tmp"; trap - 0 1 2 3 15

${SYSTEM_COMMAND_INDEX}: ${SYSTEM_MANUAL_SRCS}
	@set -eu; mkdir -p ${MANUAL_BUILD}; tmp='$@.txt.$$$$'; trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	: > "$$tmp"; \
	for f in ${SYSTEM_MANUAL_SRCS}; do \
		sed -n 's/^# @?\([^ ]*\) - .*/\1/p' "$$f" | head -n 1 >> "$$tmp"; \
	done; \
	awk 'BEGIN { subcmd=0 } \
		/^## @\?SUBCOMMANDS$$/ { subcmd=1; next } \
		subcmd && /^## / { subcmd=0 } \
		subcmd && /^### @\?/ { x=substr($$0,7); sub(/ .*/,"",x); print x }' \
		${PROJECT_ROOT}/userland/manual/DSH.SIXMD >> "$$tmp"; \
	sort -u "$$tmp" -o "$$tmp"; \
	${S6FILTER} "$$tmp" > $@; \
	rm -f "$$tmp"; trap - 0 1 2 3 15

${DAS_NATIVE_STAMP}: \
	${DAS_REPO}/Makefile ${DAS_REPO}/das.c ${DAS_REPO}/das_native1.c \
	${DAS_REPO}/das_native2.c ${DAS_REPO}/das_native_driver.c \
	${DAS_REPO}/das_native_runtime.h ${DAS_MANUAL} ${PDP10_KCC}
	mkdir -p ${DAS_NATIVE_BUILD}
	${MAKE} -C ${DAS_REPO} native \
	    PDP10_PREFIX='${PDP10_PREFIX}' DAIMOS_REPO='${PROJECT_ROOT}' \
	    NATIVE_BUILD_DIR='${DAS_NATIVE_BUILD}'
	touch $@

${DAS_NATIVE_DRIVER} ${DAS_NATIVE_PHASE1} ${DAS_NATIVE_PHASE2}: ${DAS_NATIVE_STAMP}
	@test -f $@

${DAIMOS_TOOLS_NATIVE_STAMP}: \
	${DAIMOS_TOOLS_REPO}/Makefile ${DAIMOS_TOOLS_NATIVE_INPUTS} \
	${DAIMOS_TOOLS_MANUALS} ${PDP10_KCC}
	mkdir -p ${DAIMOS_TOOLS_NATIVE_BUILD}
	${MAKE} -C ${DAIMOS_TOOLS_REPO} native \
	    PDP10_PREFIX='${PDP10_PREFIX}' DAIMOS_REPO='${PROJECT_ROOT}' \
	    NATIVE_BUILD_DIR='${DAIMOS_TOOLS_NATIVE_BUILD}'
	touch $@

${DAIMOS_NATIVE_DARC} ${DAIMOS_NATIVE_DLINK} ${DAIMOS_NATIVE_OBJDUMP}: ${DAIMOS_TOOLS_NATIVE_STAMP}
	@test -f $@

${KCC_NATIVE_STAMP}: ${KCC_REPO}/Makefile ${KCC_NATIVE_INPUTS}
	mkdir -p ${KCC_NATIVE_BUILD}
	${MAKE} -C ${KCC_REPO} kcc
	${MAKE} -C ${KCC_REPO} native-phase-dxrs native-driver \
	    PDP10_PREFIX='${PDP10_PREFIX}' DAIMOS_REPO='${PROJECT_ROOT}' \
	    KCC='${KCC_REPO}/kcc' NATIVE_BUILD_DIR='${KCC_NATIVE_BUILD}'
	touch $@

${KCC_NATIVE_DRIVER} ${KCC_NATIVE_KCPP} ${KCC_NATIVE_KPARSE} ${KCC_NATIVE_KGEN} ${KCC_NATIVE_KOPT}: ${KCC_NATIVE_STAMP}
	@test -f $@

${USERLAND_STAMP}: ${USERLAND_INPUTS} ${PDP10_KCC}
	${MAKE} -C ${PROJECT_ROOT}/userland build BUILD_ROOT='${USERLAND_BUILD_ROOT}' \
	    PDP10_PREFIX='${PDP10_PREFIX}'
	@dsh_image=`${DXRCHECK} ${DSH_DXR} | \
	    sed -n 's/^DXR image=\([0-7][0-7]*\).*/\1/p'`; \
	    test -n "$$dsh_image" || { \
	        echo "cannot determine DSH image size" >&2; exit 1; }; \
	    test "$$dsh_image" -le 036000 || { \
	        echo "DSH image $$dsh_image exceeds EXEC limit 036000" >&2; \
	        exit 1; \
	    }
	touch $@

${INIT_DXR} ${MOUNTALL_DXR} ${LOGIN_DXR} ${DSH_DXR} ${DSHCOMP_DXR} ${SYSTEM_LIBRARY_COMMAND_DXRS} ${XARGS_DXR} ${SED_DXR} ${CONFIGCTL_DXR} ${MAKE_DXR} ${TSFSPROBE_DXR} ${MOUNTTSFS_DXR} ${DTFSPROBE_DXR} ${D6LZ_DXR} ${LOGDRAIN_DXR}: ${USERLAND_STAMP}
	@test -f $@

${USERLAND_D6LZ_STAMP}: ${USERLAND_STAMP}
	@set -eu; \
	for base in login dsh xargs sed configctl make; do \
		${D6LZ_HOST} -x "${USERLAND_BUILD_ROOT}/userland/$$base.dxr" \
		    "${USERLAND_BUILD_ROOT}/userland/$$base.d6lz.dxr"; \
	done; \
	for n in ${SYSTEM_LIBRARY_COMMAND_NAMES}; do \
		${D6LZ_HOST} -x "${USERLAND_BUILD_ROOT}/userland/program/$$n.dxr" \
		    "${USERLAND_BUILD_ROOT}/userland/program/$$n.d6lz.dxr"; \
	done; \
	touch '$@'

${LOGIN_D6LZ_DXR} ${DSH_D6LZ_DXR} ${XARGS_D6LZ_DXR} ${SED_D6LZ_DXR} \
${CONFIGCTL_D6LZ_DXR} ${MAKE_D6LZ_DXR} ${SYSTEM_LIBRARY_COMMAND_D6LZ_DXRS}: ${USERLAND_D6LZ_STAMP}
	@test -f $@

${IMAGE_DXR}: ${MINIT_TABLE_OBJ} ${KCORE_EMBED_OBJ} ${KINIT_OBJS} \
    ${MRES_PACKAGE_OBJS} ${KINIT_LATE_OBJS} ${END_OBJ} ${KCORE_MAP}
	${DLINK} -b 030000 -A ${KCORE_MAP} -o $@ -M ${IMAGE_MAP} \
	    ${MINIT_TABLE_OBJ} ${KCORE_EMBED_OBJ} ${KINIT_OBJS} \
	    ${MRES_PACKAGE_OBJS} ${KINIT_LATE_OBJS} ${END_OBJ}

${IMAGE_MAP}: ${IMAGE_DXR}
	@test -f $@

${IMAGE_RIM}: ${IMAGE_DXR}
	${DXRCONVERT} --simh -b 030000 $< $@

${IMAGE_RAW}: ${IMAGE_RIM} ${IMAGE_MAP}
	@end=; \
	while read name value rest; do \
		if test "$$name" = "__kinit_image_end"; then \
			end=$$value; \
			break; \
		fi; \
	done < ${IMAGE_MAP}; \
	test -n "$$end"; words=$$((0$$end - 030000)); words=`printf '%o' "$$words"`; \
	${MKSTREAM} -i ${IMAGE_RIM} -o $@ -b 030000 -w "$$words"

${IMAGE_WORDS}: ${IMAGE_RAW} ${IMAGE_MAP} ${KCORE_STREAM} ${KCORE_MAP} image/mkimage.sh
	./image/mkimage.sh --map ${IMAGE_MAP} --input ${IMAGE_RAW} \
	    --kcore ${KCORE_STREAM} --kcore-map ${KCORE_MAP} \
	    --core-words ${BOOT_CORE_WORDS} \
	    --stack-words ${KINIT_STACK_RESERVE_WORDS} --output $@

${IMAGE_LOWMEM_WORDS}: ${IMAGE_WORDS}
	@set -eu; \
	tmp='$@.tmp.$$$$'; \
	trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	{ head -n 2 ${IMAGE_WORDS}; printf '%012o\n' 0; tail -n +3 ${IMAGE_WORDS}; } > "$$tmp"; \
	mv "$$tmp" $@; \
	trap - 0 1 2 3 15

${IMAGE_COMPRESSED_WORDS}: ${IMAGE_WORDS}
	@set -eu; \
	tmp_raw='$@.raw.$$$$'; tmp_comp='$@.comp.$$$$'; tmp_out='$@.tmp.$$$$'; \
	trap 'rm -f "$$tmp_raw" "$$tmp_comp" "$$tmp_out"' 0 1 2 3 15; \
	tail -n +3 ${IMAGE_WORDS} > "$$tmp_raw"; \
	${D6LZ_HOST} -t "$$tmp_raw" "$$tmp_comp"; \
	cwords=$$(wc -l < "$$tmp_comp" | tr -d ' '); \
	desc=$$(sed -n '2p' ${IMAGE_WORDS}); \
	uwords=$$((0$${desc%??????})); \
	stage_end=$$((030000 + uwords + cwords)); \
	core_words=$$((0${BOOT_CORE_WORDS})); \
	if [ $$stage_end -gt $$core_words ]; then \
		echo "compressed boot image exceeds configured core: stage_end=$$(printf '%o' $$stage_end) core=$$(printf '%o' $$core_words)" >&2; \
		exit 1; \
	fi; \
	{ head -n 2 ${IMAGE_WORDS}; printf '%06o%06o\n' $$cwords 0; cat "$$tmp_comp"; } > "$$tmp_out"; \
	mv "$$tmp_out" $@; \
	trap - 0 1 2 3 15

${STAGE1_DXR}: ${STAGE1_SRC} ${STAGE1_COMMON} ${BUILD}
	${DAS} ${DASFLAGS} -K -O $@ $<

${STAGE1_TAPE}: ${STAGE1_DXR}
	${DXRCONVERT} --pdp6-readin -b 060 $< $@
	printf '\252' >> $@

D6FS_ROOT_DEPS = ${IMAGE_BOOT_WORDS} ${SYSTEM_INIT_DXR} ${SYSTEM_MOUNTALL_DXR} \
	${SYSTEM_LOGIN_DXR} ${SYSTEM_DSH_DXR} ${DSHCOMP_DXR} ${SYSTEM_LIBRARY_COMMAND_D6LZ_DXRS} \
	${SYSTEM_XARGS_DXR} ${SYSTEM_SED_DXR} \
	${SYSTEM_CONFIGCTL_DXR} ${SYSTEM_MAKE_DXR} ${SYSTEM_TSFSPROBE_DXR} \
	${SYSTEM_MOUNTTSFS_DXR} ${SYSTEM_DTFSPROBE_DXR} ${SYSTEM_D6LZ_DXR} \
	${SYSTEM_LOGDRAIN_DXR} ${DAS_NATIVE_DRIVER} ${DAS_NATIVE_PHASE1} \
	${DAS_NATIVE_PHASE2} ${DAIMOS_NATIVE_DARC} ${DAIMOS_NATIVE_DLINK} \
	${DAIMOS_NATIVE_OBJDUMP} ${SYSTEM_CONFIG_S6REC} \
	${SYSTEM_MANUALS} ${ASMUTILS_SOURCE_S6REC} ${PICTURE_FILES}

KCC_D6FS_DISK_DEPS = ${KCC_NATIVE_DRIVER} ${KCC_NATIVE_KCPP} \
	${KCC_NATIVE_KPARSE} ${KCC_NATIVE_KGEN} ${KCC_NATIVE_KOPT}

KCC_D6FS_DISK_ARGS = \
	-D /OPTION/BASE/LIBEXEC/KCC:0555 \
	-f "/OPTION/BASE/EXEC/KCC:${KCC_NATIVE_DRIVER}:555:dxr" \
	-f "/OPTION/BASE/LIBEXEC/KCC/KCPP:${KCC_NATIVE_KCPP}:555:dxr" \
	-f "/OPTION/BASE/LIBEXEC/KCC/KPARSE:${KCC_NATIVE_KPARSE}:555:dxr" \
	-f "/OPTION/BASE/LIBEXEC/KCC/KGEN:${KCC_NATIVE_KGEN}:555:dxr" \
	-f "/OPTION/BASE/LIBEXEC/KCC/KOPT:${KCC_NATIVE_KOPT}:555:dxr"

D6FS_ROOT_ARGS = -D /TEMP:0777 -D /SCRATCH:0777 \
	-D /MOUNT/DT0:0755 -D /MOUNT/DT1:0755 -D /MOUNT/DT2:0755 \
	-D /SYSTEM:0555 -D /SYSTEM/EXEC:0555 -D /SYSTEM/LIBEXEC:0555 -D /SYSTEM/LIB:0555 \
	-D /SYSTEM/INCLUDE:0555 -D /SYSTEM/MANUAL:0555 -D /SYSTEM/SOURCE:0555 \
	-D /OPTION:0555 -D /OPTION/BASE:0555 -D /OPTION/BASE/EXEC:0555 \
	-D /OPTION/BASE/LIBEXEC:0555 -D /OPTION/BASE/LIBEXEC/DAS:0555 \
	-D /OPTION/BASE/LIB:0555 -D /OPTION/BASE/INCLUDE:0555 \
	-D /OPTION/BASE/MANUAL:0555 -D /OPTION/BASE/SOURCE:0555 \
	-D /OPTION/BASE/SOURCE/ASMUTILS:0755 \
	-D /PICTURES:0555 \
	-D /CONFIG:0755 \
	-f "/SYSTEM/INIT:${SYSTEM_INIT_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/MOUNTALL:${SYSTEM_MOUNTALL_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/LOGIN:${SYSTEM_LOGIN_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/DSH:${SYSTEM_DSH_DXR}:555:dxr" \
	-f "/SYSTEM/LIBEXEC/DSHCOMP:${DSHCOMP_DXR}:555:dxr" \
	${SYSTEM_LIBRARY_COMMAND_ARGS} \
	-f "/SYSTEM/EXEC/XARGS:${SYSTEM_XARGS_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/SED:${SYSTEM_SED_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/CONFIGCTL:${SYSTEM_CONFIGCTL_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/MAKE:${SYSTEM_MAKE_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/TSFSPROBE:${SYSTEM_TSFSPROBE_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/MOUNT.TSFS:${SYSTEM_MOUNTTSFS_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/DTFSPROBE:${SYSTEM_DTFSPROBE_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/D6LZ:${SYSTEM_D6LZ_DXR}:555:dxr" \
	-f "/SYSTEM/EXEC/LOGDRAIN:${SYSTEM_LOGDRAIN_DXR}:555:dxr" \
	-f "/OPTION/BASE/EXEC/DAS:${DAS_NATIVE_DRIVER}:555:dxr" \
	-f "/OPTION/BASE/LIBEXEC/DAS/DAS1:${DAS_NATIVE_PHASE1}:555:dxr" \
	-f "/OPTION/BASE/LIBEXEC/DAS/DAS2:${DAS_NATIVE_PHASE2}:555:dxr" \
	-f "/OPTION/BASE/EXEC/DARC:${DAIMOS_NATIVE_DARC}:555:dxr" \
	-f "/OPTION/BASE/EXEC/DLINK:${DAIMOS_NATIVE_DLINK}:555:dxr" \
	-f "/OPTION/BASE/EXEC/OBJDUMP:${DAIMOS_NATIVE_OBJDUMP}:555:dxr" \
	${SYSTEM_MANUAL_ARGS} \
	${ASMUTILS_SOURCE_ARGS} \
	-f "/CONFIG/INITTAB:${SYSTEM_INITTAB_S6REC}:644:binwords" \
	-f "/CONFIG/FSTAB:${SYSTEM_FSTAB_S6REC}:644:binwords" \
	-f "/CONFIG/PASSWD:${SYSTEM_PASSWD_S6REC}:600:binwords" \
	-f "/CONFIG/SYSTEM:${SYSTEM_SYSTEM_S6REC}:644:binwords" \
	${PICTURE_D6FS_ARGS} \
	${D6FS_EXTRA_ARGS}

${DISK_STAMP}: ${D6FS_ROOT_DEPS} ${KCC_D6FS_DISK_DEPS}
	rm -rf ${DISK_DIR}
	${MKDSK} -n 4 -m clean -p ${IMAGE_BOOT_WORDS} -o ${DISK_DIR} \
	    --d6fs-layout --logstore-blocks 0 \
	    --swap-tail-blocks 0 \
	    --spare-blocks ${D6FS_SPARE_BLOCKS} \
	    --member-sectors 0130000,0130000,0130000,0130000
	${MKD6FS} -n 4 -d ${DISK_DIR} ${D6FS_ROOT_ARGS} \
	    ${KCC_D6FS_DISK_ARGS} ${D6FS_DISK_EXTRA_ARGS}
	touch $@

${DISK_IMAGES}: ${DISK_STAMP}
	@test -f $@

${DRUM_STAMP}: ${D6FS_ROOT_DEPS}
	rm -rf ${DRUM_DIR}
	${MKDSK} -n 4 -m clean -p ${IMAGE_BOOT_WORDS} -o ${DRUM_DIR} \
	    --d6fs-layout --logstore-blocks 0 \
	    --swap-tail-blocks ${D6FS_SWAP_TAIL_BLOCKS} \
	    --spare-blocks ${D6FS_SPARE_BLOCKS}
	${MKD6FS} -n 4 -d ${DRUM_DIR} ${D6FS_ROOT_ARGS}
	# D6SET1 has an ordinary D6FS data area, while the physical DRM0
	# tail contains independent raw AUXSTORE LOGSTORE/BACKSTORE regions.
	# MKAUXSTORE preserves the existing D6FS/blockset media descriptor.
	${MKAUXSTORE} -i ${DR0_IMAGE} --from-d6fs-layout \
	    --logstore-blocks ${DRUM_LOGSTORE_BLOCKS} \
	    --backstore-blocks ${DRUM_BACKSTORE_BLOCKS} --cache-blocks 0
	touch $@

${DRUM_IMAGES}: ${DRUM_STAMP}
	@test -f $@

TSFS_ROOT_COMMAND_NAMES = ECHO CAT LS MKDIR RM PWD STAT TOUCH CP MV HEXDUMP PS \
	DEVS FREE MEMSTAT DATE RMDIR CHOWN CHMOD MKFS.DTFS FSCK.DTFS MOUNT \
	MOUNT.DTFS UNMOUNT DF TTYOUT HALT
TSFS_ROOT_COMMAND_DXRS != for n in ${TSFS_ROOT_COMMAND_NAMES}; do printf '%s ' "${USERLAND_BUILD_ROOT}/userland/program/$$n.d6lz.dxr"; done

${TSFS_ROOT_IMAGE}: ${INIT_DXR} ${SYSTEM_MOUNTALL_DXR} ${LOGIN_DXR} ${DSH_DXR} ${TSFS_ROOT_COMMAND_DXRS} ${DTFSPROBE_DXR} \
	${SYSTEM_CONFIG_S6REC} ${PICTURE_FILES}
	@set -eu; mkdir -p ${MEDIA_DIR}; \
	tmp='${TSFS_ROOT_MANIFEST}.tmp.$$$$'; \
	trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	{ \
	  echo 'D /MOUNT 0755 0 0'; \
	  echo 'D /TEMP 0777 0 0'; \
	  echo 'D /SCRATCH 0777 0 0'; \
	  echo 'D /SYSTEM 0555 0 0'; \
	  echo 'D /SYSTEM/EXEC 0555 0 0'; \
	  echo 'D /PICTURES 0555 0 0'; \
	  echo 'D /CONFIG 0755 0 0'; \
	  printf 'F /SYSTEM/INIT %s 0555 0 0\n' '${INIT_DXR}'; \
	  printf 'F /SYSTEM/EXEC/MOUNTALL %s 0555 0 0\n' '${SYSTEM_MOUNTALL_DXR}'; \
	  printf 'F /CONFIG/FSTAB %s 0644 0 0\n' '${SYSTEM_FSTAB_S6REC}'; \
	  printf 'F /CONFIG/INITTAB %s 0644 0 0\n' '${SYSTEM_INITTAB_S6REC}'; \
	  printf 'F /SYSTEM/EXEC/LOGIN %s 0555 0 0\n' '${LOGIN_DXR}'; \
	  printf 'F /CONFIG/PASSWD %s 0600 0 0\n' '${SYSTEM_PASSWD_S6REC}'; \
	  printf 'F /CONFIG/SYSTEM %s 0644 0 0\n' '${SYSTEM_SYSTEM_S6REC}'; \
	  printf 'F /SYSTEM/EXEC/DSH %s 0555 0 0\n' '${DSH_DXR}'; \
	  for cmd in ${TSFS_ROOT_COMMAND_NAMES}; do \
	    printf 'F /SYSTEM/EXEC/%s %s 0555 0 0\n' "$$cmd" "${USERLAND_BUILD_ROOT}/userland/program/$$cmd.d6lz.dxr"; \
	  done; \
	  for src in ${PICTURE_FILES}; do \
	    leaf=$$(basename "$$src" | tr '[:lower:]' '[:upper:]'); \
	    printf 'F /PICTURES/%s %s 0444 0 0\n' "$$leaf" "$$src"; \
	  done; \
	  printf 'F /SYSTEM/EXEC/DTFSPROBE %s 0555 0 0\n' '${DTFSPROBE_DXR}'; \
	} > "$$tmp"; \
	mv "$$tmp" '${TSFS_ROOT_MANIFEST}'; \
	trap - 0 1 2 3 15
	${MKTSFS} -n 1 -i 1:1 -g 1 -m '${TSFS_ROOT_MANIFEST}' \
	    -o '${TSFS_ROOT_PREFIX}'

${BOOT_MEDIA_STAMP}: ${IMAGE_BOOT_WORDS} ${DISK_IMAGES} ${DRUM_IMAGES} \
	${STAGE1_TAPE} ${ROOT_MEDIA_DEPS}
	@set -eu; mkdir -p ${BUILD}; \
	if [ '${BOOT_PROFILE}' = lowmem ] && [ '${STAGE1_DEVICE}' != dsk ]; then \
		echo 'BOOT_PROFILE=lowmem currently requires BOOT=dsk' >&2; exit 2; \
	fi; \
	mkdir -p '${MEDIA_DIR}'; \
	touch '${DTC0_IMAGE}' '${DTC1_IMAGE}' '${DTC2_IMAGE}' '${MTC0_IMAGE}' '${MTC1_IMAGE}'; \
	case '${STAGE1_DEVICE}' in \
	  dsk|drm) : ;; \
	  dtc) '${MKDT}' -t dtc -p '${IMAGE_COMPRESSED_WORDS}' -o '${DTC0_IMAGE}' ;; \
	  mtc) '${MKTAP}' -t mtc -p '${IMAGE_COMPRESSED_WORDS}' -o '${MTC0_IMAGE}' ;; \
	  ptr) \
	    cwords=$$(tail -n +4 '${IMAGE_COMPRESSED_WORDS}' | wc -l | tr -d ' '); \
	    first=$$(( (cwords + 1) / 2 )); second=$$(( cwords - first )); \
	    test $$first -gt 0; test $$second -gt 0; \
	    { printf '%012o\n' $$((first + 3)); head -n 3 '${IMAGE_COMPRESSED_WORDS}'; \
	      tail -n +4 '${IMAGE_COMPRESSED_WORDS}' | head -n $$first; } > '${PTR_BOOT1_WORDS}'; \
	    { printf '%012o\n' $$second; tail -n +4 '${IMAGE_COMPRESSED_WORDS}' | \
	      tail -n +$$((first + 1)); } > '${PTR_BOOT2_WORDS}'; \
	    '${WORDS2PT}' -t ptr -p '${PTR_BOOT1_WORDS}' -o '${PTR_BOOT1}'; \
	    '${WORDS2PT}' -t ptr -p '${PTR_BOOT2_WORDS}' -o '${PTR_BOOT2}' ;; \
	esac; \
	if [ '${ROOT}' = tape ]; then \
	  if [ '${STAGE1_DEVICE}' = dtc ]; then \
	    cp '${TSFS_ROOT_IMAGE}' '${DTC1_IMAGE}'; \
	  else \
	    cp '${TSFS_ROOT_IMAGE}' '${DTC0_IMAGE}'; \
	  fi; \
	fi; \
	{ \
	  if [ '${STAGE1_DEVICE}' = ptr ]; then \
	    echo 'break 060'; \
	    echo 'go 020'; \
	    echo 'nobreak 060'; \
	    echo 'detach ptr'; \
	    echo 'attach -q ptr ${PTR_BOOT1}'; \
	    echo 'deposit ptr time 1'; \
	    echo 'expect "TAPE2" detach ptr; attach -q ptr ${PTR_BOOT2}; deposit ptr time 1; continue'; \
	    echo 'go 060'; \
	  else \
	    echo 'go 020'; \
	  fi; \
	} > '${BOOT_COMMANDS}'; \
	touch '$@'

${SIMH_INI}: simh/simh.ini.in ${STAGE1_TAPE} ${DISK_IMAGES} ${DRUM_IMAGES} \
	${BOOT_MEDIA_STAMP} force-simh-config
	@case '${SIMH_PCLK_MODE}' in \
	  REALTIME|HISTORICAL) ;; \
	  *) echo "SIMH_PCLK_MODE must be REALTIME or HISTORICAL" >&2; exit 2 ;; \
	esac
	@case '${SIMH_DPY_MODE}' in \
	  GUI|HEADLESS) ;; \
	  *) echo "SIMH_DPY_MODE must be GUI or HEADLESS" >&2; exit 2 ;; \
	esac
	@case '${ROOT}' in \
	  auto) root_class=0 ;; \
	  disk) root_class=1 ;; \
	  tape) root_class=2 ;; \
	  drum) root_class=3 ;; \
	  *) echo "ROOT must be auto, disk, tape, or drum" >&2; exit 2 ;; \
	esac; \
	case '${ROOTSET}' in 1|2|3|4) ;; \
	  *) echo "ROOTSET must be 1, 2, 3, or 4" >&2; exit 2 ;; \
	esac; \
	root_switch=$$((root_class + ((${ROOTSET} - 1) << 3))); \
	printf '%o\n' $$root_switch > ${BUILD}/root-switch.tmp
	root_switch=`cat ${BUILD}/root-switch.tmp`; rm -f ${BUILD}/root-switch.tmp; \
	sed -e 's|@SIMH_LOG@|${SIMH_LOG}|g' \
	    -e 's|@CPU_KWORDS@|${SIMH_CPU_KWORDS}|g' \
	    -e 's|@STAGE1_TAPE@|${STAGE1_TAPE}|g' \
	    -e 's|@DSK0_IMAGE@|${DISK0_IMAGE}|g' \
	    -e 's|@DSK1_IMAGE@|${DISK1_IMAGE}|g' \
	    -e 's|@DSK2_IMAGE@|${DISK2_IMAGE}|g' \
	    -e 's|@DSK3_IMAGE@|${DISK3_IMAGE}|g' \
	    -e 's|@DTC0_IMAGE@|${DTC0_IMAGE}|g' \
	    -e 's|@DTC1_IMAGE@|${DTC1_IMAGE}|g' \
	    -e 's|@DTC2_IMAGE@|${DTC2_IMAGE}|g' \
	    -e 's|@MTC0_IMAGE@|${MTC0_IMAGE}|g' \
	    -e 's|@MTC1_IMAGE@|${MTC1_IMAGE}|g' \
	    -e 's|@DR0_IMAGE@|${DR0_IMAGE}|g' \
	    -e 's|@DR1_IMAGE@|${DR1_IMAGE}|g' \
	    -e 's|@DR2_IMAGE@|${DR2_IMAGE}|g' \
	    -e 's|@DR3_IMAGE@|${DR3_IMAGE}|g' \
	    -e 's|@PTP_OUTPUT@|${PTP_OUTPUT}|g' \
	    -e 's|@CP_OUTPUT@|${CP_OUTPUT}|g' \
	    -e 's|@LPT_OUTPUT@|${LPT_OUTPUT}|g' \
	    -e 's|@DCS0_PORT@|${SIMH_DCS0_PORT}|g' \
	    -e 's|@GE0_PORT@|${SIMH_GE0_PORT}|g' \
	    -e 's|@PCLK_MODE@|${SIMH_PCLK_MODE}|g' \
	    -e 's|@DPY_MODE@|${SIMH_DPY_MODE}|g' \
	    -e "s|@ROOT_SWITCH@|$$root_switch|g" $< > $@.tmp; \
	while IFS= read -r line || test -n "$$line"; do \
		if test "$$line" = "@BOOT_COMMANDS@"; then \
			cat '${BOOT_COMMANDS}'; \
		else \
			printf '%s\n' "$$line"; \
		fi; \
	done < $@.tmp > $@; \
	rm -f $@.tmp

image: check-config ${SIMH_INI}

permanent-size: ${KCORE_MAP} ${MRES_PACKAGE_OBJS}
	@./image/report-permanent.sh --kcore-map ${KCORE_MAP} --build ${BUILD} \
	    --objdump ${PDP10_OBJDUMP} --omit-blockset

boot:
	rm -rf ${DISK_DIR} ${DRUM_DIR} ${SIMH_INI}
	${MAKE} image SIMH_DPY_MODE=GUI DPY_LOGIN=0
	${SIMH_PDP6} ${SIMH_INI}

clean:
	rm -rf ${BUILD}
