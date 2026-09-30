.PHONY: force-minit-config
PROJECT_ROOT != cd ../../.. && pwd
KERNEL_DIR = $(PROJECT_ROOT)/system/kernel
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/system/boot/pdp6
PDP10_PREFIX ?= $(HOME)/cross

DAS ?= $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -O
DLINK ?= $(PDP10_PREFIX)/bin/dlink
P10FOLD ?= $(PDP10_PREFIX)/bin/p10fold
DXRCONVERT ?= $(PDP10_PREFIX)/bin/dxrconvert
MKSTREAM ?= $(PDP10_PREFIX)/bin/mkstream
D6LZ_HOST ?= $(PDP10_PREFIX)/bin/d6lz
MKDSK ?= $(PDP10_PREFIX)/bin/mkdsk
MKD6FS ?= $(PDP10_PREFIX)/bin/mkd6fs
MKDT ?= $(PDP10_PREFIX)/bin/mkdt
MKTSFS ?= $(PDP10_PREFIX)/bin/mktsfs
MKTAP ?= $(PDP10_PREFIX)/bin/mktap
WORDS2PT ?= $(PDP10_PREFIX)/bin/words2pt
S6FILTER ?= $(PDP10_PREFIX)/bin/s6filter
S6TEXT ?= $(PDP10_PREFIX)/bin/s6text
LOGSTORE ?= $(PDP10_PREFIX)/bin/logstore
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
ROOT ?= auto
ROOTSET ?= 1
BOOT ?= auto
BOOT_PROFILE ?= full
ifneq ($(filter $(BOOT_PROFILE),full lowmem),$(BOOT_PROFILE))
$(error unsupported BOOT_PROFILE=$(BOOT_PROFILE); expected full or lowmem)
endif
KINIT_STACK_RESERVE_WORDS ?= 02000
ifeq ($(BOOT_PROFILE),lowmem)
BOOT_CORE_WORDS ?= 0100000
SIMH_CPU_KWORDS ?= 32
KINIT_BADMAP ?= 0
else
BOOT_CORE_WORDS ?= 01000000
SIMH_CPU_KWORDS ?= 256
KINIT_BADMAP ?= 1
endif
KINIT_STACK_WATERMARK ?= 0
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
PDP10_OBJDUMP ?= $(PDP10_PREFIX)/bin/pdp10-objdump

D6FS_LOGSTORE_BLOCKS ?= 0400
D6FS_SWAP_TAIL_BLOCKS ?= 0200
D6FS_SPARE_BLOCKS ?= 0
PROC_BOOT_USERS ?= 1
PROC_STACK_WATERMARK ?= 0
VM_BACKEND ?= pdp6

KINIT_BUILD = $(BUILD)/kinit-asm
KCORE_BUILD = $(BUILD)/kcore-asm
KINIT_STAMP = $(BUILD)/.kinit-built
KCORE_STAMP = $(BUILD)/.kcore-built
KERNEL_INPUTS = $(KERNEL_DIR)/Makefile \
	$(PROJECT_ROOT)/system/stand/pdp6/common/decompressor.inc \
	$(KERNEL_DIR)/boot/kinit.c \
	$(KERNEL_DIR)/boot/kinit.h \
	$(KERNEL_DIR)/boot/kinit_boot.c \
	$(KERNEL_DIR)/boot/kinit_diag.c \
	$(KERNEL_DIR)/boot/kinit_io_pdp6.s \
	$(KERNEL_DIR)/boot/kinit_late.c \
	$(KERNEL_DIR)/boot/kinit_late_begin_pdp6.s \
	$(KERNEL_DIR)/boot/kinit_late_end.s \
	$(KERNEL_DIR)/boot/root_select.c \
	$(KERNEL_DIR)/boot/root_select.h \
	$(KERNEL_DIR)/core/d6lz.h \
	$(KERNEL_DIR)/core/d6lz_pdp6.s \
	$(KERNEL_DIR)/core/kcore.h \
	$(KERNEL_DIR)/core/kcore_end.s \
	$(KERNEL_DIR)/core/kcore_io_pdp6.s \
	$(KERNEL_DIR)/core/kcore_pi.h \
	$(KERNEL_DIR)/core/kfmt.h \
	$(KERNEL_DIR)/core/kfmt_pdp6.s \
	$(KERNEL_DIR)/core/ret.s \
	$(KERNEL_DIR)/drivers/card.h \
	$(KERNEL_DIR)/drivers/clk.h \
	$(KERNEL_DIR)/drivers/clk_io.s \
	$(KERNEL_DIR)/drivers/cp_io.s \
	$(KERNEL_DIR)/drivers/cr_io.s \
	$(KERNEL_DIR)/drivers/cty.h \
	$(KERNEL_DIR)/drivers/cty_io.s \
	$(KERNEL_DIR)/drivers/dcs.h \
	$(KERNEL_DIR)/drivers/dcs_io.s \
	$(KERNEL_DIR)/drivers/device_state.s \
	$(KERNEL_DIR)/drivers/dpy.h \
	$(KERNEL_DIR)/drivers/dpy_io.s \
	$(KERNEL_DIR)/drivers/dsk270.h \
	$(KERNEL_DIR)/drivers/drm236.h \
	$(KERNEL_DIR)/drivers/dsk270.s \
	$(KERNEL_DIR)/drivers/drm236.s \
	$(KERNEL_DIR)/drivers/dsk_io.s \
	$(KERNEL_DIR)/drivers/drm236_io.s \
	$(KERNEL_DIR)/drivers/ge.h \
	$(KERNEL_DIR)/drivers/ge_io.s \
	$(KERNEL_DIR)/drivers/lpt.h \
	$(KERNEL_DIR)/drivers/lpt_io.s \
	$(KERNEL_DIR)/drivers/ocnsls.h \
	$(KERNEL_DIR)/drivers/ocnsls_io.s \
	$(KERNEL_DIR)/drivers/pt.h \
	$(KERNEL_DIR)/drivers/pclk.h \
	$(KERNEL_DIR)/drivers/ptp_io.s \
	$(KERNEL_DIR)/drivers/ptr_io.s \
	$(KERNEL_DIR)/drivers/slv.h \
	$(KERNEL_DIR)/drivers/slv_io.s \
	$(KERNEL_DIR)/drivers/tape_io.s \
	$(KERNEL_DIR)/drivers/tty.h \
	$(KERNEL_DIR)/drivers/tty_io.s \
	$(KERNEL_DIR)/drivers/wcnsls.h \
	$(KERNEL_DIR)/drivers/wcnsls_io.s \
	$(KERNEL_DIR)/fs/d6fs.c \
	$(KERNEL_DIR)/fs/d6fs.h \
	$(KERNEL_DIR)/boot/d6fs_boot.c \
	$(KERNEL_DIR)/boot/d6fs_boot.h \
	$(KERNEL_DIR)/fs/d6fs_pdp10.s \
	$(KERNEL_DIR)/fs/d6fs_provider.c \
	$(KERNEL_DIR)/fs/d6fs_provider.h \
	$(KERNEL_DIR)/fs/d6fs_provider_mutation_pdp10.s \
	$(KERNEL_DIR)/boot/d6fs_super_boot.c \
	$(KERNEL_DIR)/fs/d6fs_validate_pdp10.s \
	$(KERNEL_DIR)/fs/monitorfs.h \
	$(KERNEL_DIR)/fs/dtfs.c \
	$(KERNEL_DIR)/fs/tsfs.c \
	$(KERNEL_DIR)/fs/dtfs.h \
	$(KERNEL_DIR)/fs/dtfs_media.h \
	$(KERNEL_DIR)/fs/dtfs_chain_walk_pdp10.s \
	$(KERNEL_DIR)/fs/dtfs_pdp10.s \
	$(KERNEL_DIR)/fs/tsfs_pdp10.s \
	$(KERNEL_DIR)/fs/dtfs_resize_pdp10.s \
	$(KERNEL_DIR)/fs/file.c \
	$(KERNEL_DIR)/fs/file.h \
	$(KERNEL_DIR)/fs/file_pdp10.s \
	$(KERNEL_DIR)/fs/monitorfs_pdp10.s \
	$(KERNEL_DIR)/fs/fs_mres.h \
	$(KERNEL_DIR)/fs/fs_mres_pdp10.s \
	$(KERNEL_DIR)/fs/memfs.c \
	$(KERNEL_DIR)/fs/memfs.h \
	$(KERNEL_DIR)/fs/memfs_pdp10.s \
	$(KERNEL_DIR)/fs/vfs.h \
	$(KERNEL_DIR)/fs/vfs_pdp10.s \
	$(KERNEL_DIR)/ipc/pipe.h \
	$(KERNEL_DIR)/ipc/pipe_pdp10.s \
	$(KERNEL_DIR)/mm/mm.c \
	$(KERNEL_DIR)/mm/mm.h \
	$(KERNEL_DIR)/mm/mm_boot.c \
	$(KERNEL_DIR)/mm/mm_internal.h \
	$(KERNEL_DIR)/mm/mres.h \
	$(KERNEL_DIR)/mm/mres_reloc.h \
	$(KERNEL_DIR)/modules/module.h \
	$(KERNEL_DIR)/modules/module_io.s \
	$(KERNEL_DIR)/modules/module_minit.c \
	$(KERNEL_DIR)/modules/module_runtime.c \
	$(KERNEL_DIR)/modules/module_runtime.h \
	$(KERNEL_DIR)/modules/module_table.s \
	$(KERNEL_DIR)/proc/exec.c \
	$(KERNEL_DIR)/proc/exec.h \
	$(KERNEL_DIR)/proc/mach_user.h \
	$(KERNEL_DIR)/proc/mach_user.s \
	$(KERNEL_DIR)/proc/proc.c \
	$(KERNEL_DIR)/proc/proc.h \
	$(KERNEL_DIR)/proc/proc_boot.c \
	$(KERNEL_DIR)/proc/proc_late.c \
	$(KERNEL_DIR)/proc/proc_pdp10.s \
	$(KERNEL_DIR)/proc/proc_run_pdp10.s \
	$(KERNEL_DIR)/proc/proc_swap.h \
	$(KERNEL_DIR)/proc/proc_swap_boot.c \
	$(KERNEL_DIR)/proc/syscall.h \
	$(KERNEL_DIR)/proc/syscall_dispatch.s \
	$(KERNEL_DIR)/proc/vm.h \
	$(KERNEL_DIR)/proc/vm_$(VM_BACKEND).h \
	$(KERNEL_DIR)/proc/vm_$(VM_BACKEND).c \
	$(KERNEL_DIR)/proc/vm_$(VM_BACKEND)_swap.c \
	$(KERNEL_DIR)/proc/vm_$(VM_BACKEND)_asm.s \
	$(KERNEL_DIR)/proc/syscall_info_pdp10.s \
	$(KERNEL_DIR)/drivers/pclk.s \
	$(KERNEL_DIR)/storage/blockset.h \
	$(KERNEL_DIR)/storage/blockset_boot.c \
	$(KERNEL_DIR)/storage/blockset_boot.h \
	$(KERNEL_DIR)/storage/blockset_dispatch.s \
	$(KERNEL_DIR)/storage/badmap_dispatch.s \
	$(KERNEL_DIR)/storage/blockset_layout.h \
	$(KERNEL_DIR)/storage/blockset_mres.h \
	$(KERNEL_DIR)/storage/logstore.c \
	$(KERNEL_DIR)/storage/logstore_boot.c \
	$(KERNEL_DIR)/storage/logstore.h \
	$(KERNEL_DIR)/storage/logstore_drain.c \
	$(KERNEL_DIR)/storage/logstore_mtc_sink.c \
	$(KERNEL_DIR)/storage/storage.h \
	$(KERNEL_DIR)/storage/storage_router.s

KINIT_NAMES = kinit kinit_diag module_minit kinit_boot root_select d6fs_boot d6fs_super_boot blockset_boot logstore_boot logstore proc_boot proc_swap_boot kinit_io_pdp6 module_io bootbanner_generated
KINIT_LATE_NAMES = kinit_late_begin_pdp6 proc_late mm_boot kinit_late kinit_late_end
KINIT_ASM = $(KINIT_BUILD)/kinit.s $(KINIT_BUILD)/kinit_diag.s $(KINIT_BUILD)/module_minit.s $(KINIT_BUILD)/kinit_boot.s $(KINIT_BUILD)/root_select.s $(KINIT_BUILD)/d6fs_boot.s $(KINIT_BUILD)/d6fs_super_boot.s $(KINIT_BUILD)/blockset_boot.s $(KINIT_BUILD)/logstore_boot.s $(KINIT_BUILD)/proc_boot.s $(KINIT_BUILD)/proc_swap_boot.s $(KINIT_BUILD)/kinit_io_pdp6.s $(KINIT_BUILD)/module_io.s $(KINIT_BUILD)/bootbanner_generated.s
KINIT_LATE_ASM = $(KINIT_BUILD)/kinit_late_begin_pdp6.s $(KINIT_BUILD)/proc_late.s $(KINIT_BUILD)/mm_boot.s $(KINIT_BUILD)/kinit_late.s $(KINIT_BUILD)/kinit_late_end.s
KINIT_OBJS = $(BUILD)/kinit.dobj $(BUILD)/kinit_diag.dobj $(BUILD)/module_minit.dobj $(BUILD)/kinit_boot.dobj $(BUILD)/root_select.dobj $(BUILD)/tsfs_boot.dobj $(BUILD)/d6fs_boot.dobj $(BUILD)/d6fs_super_boot.dobj $(BUILD)/blockset_boot.dobj $(BUILD)/logstore_boot.dobj $(BUILD)/logstore.dobj $(BUILD)/proc_boot.dobj $(BUILD)/proc_swap_boot.dobj $(BUILD)/kinit_io_pdp6.dobj $(BUILD)/module_io.dobj $(BUILD)/bootbanner_generated.dobj
KINIT_LATE_OBJS = $(BUILD)/kinit_late_begin_pdp6.dobj $(BUILD)/proc_late.dobj $(BUILD)/mm_boot.dobj $(BUILD)/kinit_late.dobj $(BUILD)/kinit_late_end.dobj

KCORE_C_NAMES = exec d6lz vfs mm proc vm_backend proc_swap file
KCORE_FIXED_NAMES = $(KCORE_C_NAMES) ret \
	dsk270 drm236 kfmt_pdp6 vfs_pdp10 fs_mres_pdp10 file_pdp10 monitorfs_pdp10 proc_pdp10 proc_run_pdp10 \
	device_state syscall_info_pdp10 pclk lpt_io \
	syscall_dispatch mach_user vm_backend_asm kcore_io_pdp6 storage_router kcore_end
KCORE_FIXED_ASM = $(KCORE_BUILD)/d6lz_fixed.s $(KCORE_BUILD)/exec.s $(KCORE_BUILD)/d6lz.s $(KCORE_BUILD)/mm.s $(KCORE_BUILD)/proc.s $(KCORE_BUILD)/vm_backend.s $(KCORE_BUILD)/proc_swap.s $(KCORE_BUILD)/file.s $(KCORE_BUILD)/ret.s $(KCORE_BUILD)/dsk270.s $(KCORE_BUILD)/drm236.s $(KCORE_BUILD)/kfmt_pdp6.s $(KCORE_BUILD)/vfs_pdp10.s $(KCORE_BUILD)/fs_mres_pdp10.s $(KCORE_BUILD)/file_pdp10.s $(KCORE_BUILD)/pipe_pdp10.s $(KCORE_BUILD)/monitorfs_pdp10.s $(KCORE_BUILD)/proc_pdp10.s $(KCORE_BUILD)/proc_run_pdp10.s $(KCORE_BUILD)/device_state.s $(KCORE_BUILD)/syscall_info_pdp10.s $(KCORE_BUILD)/pclk.s $(KCORE_BUILD)/lpt_io.s $(KCORE_BUILD)/syscall_dispatch.s $(KCORE_BUILD)/mach_user.s $(KCORE_BUILD)/vm_backend_asm.s $(KCORE_BUILD)/kcore_io_pdp6.s $(KCORE_BUILD)/storage_router.s $(KCORE_BUILD)/kcore_end.s
KCORE_FIXED_OBJS = $(BUILD)/d6lz_fixed.dobj $(BUILD)/exec.dobj $(BUILD)/d6lz.dobj $(BUILD)/mm.dobj $(BUILD)/proc.dobj $(BUILD)/vm_backend.dobj $(BUILD)/proc_swap.dobj $(BUILD)/file.dobj $(BUILD)/ret.dobj $(BUILD)/dsk270.dobj $(BUILD)/drm236.dobj $(BUILD)/kfmt_pdp6.dobj $(BUILD)/vfs_pdp10.dobj $(BUILD)/fs_mres_pdp10.dobj $(BUILD)/file_pdp10.dobj $(BUILD)/pipe_pdp10.dobj $(BUILD)/monitorfs_pdp10.dobj $(BUILD)/proc_pdp10.dobj $(BUILD)/proc_run_pdp10.dobj $(BUILD)/device_state.dobj $(BUILD)/syscall_info_pdp10.dobj $(BUILD)/pclk.dobj $(BUILD)/lpt_io.dobj $(BUILD)/syscall_dispatch.dobj $(BUILD)/mach_user.dobj $(BUILD)/vm_backend_asm.dobj $(BUILD)/kcore_io_pdp6.dobj $(BUILD)/storage_router.dobj $(BUILD)/kcore_end.dobj
MRES_NAMES = cty_io clk_io ptr_io ptp_io cr_io cp_io dcs_io ge_io dpy_io \
	tty_io wcnsls_io ocnsls_io dsk_io drm236_io tape_io slv_io memfs_pdp10 \
	dtfs dtfs_pdp10 tsfs tsfs_pdp10 blockset_dispatch badmap_dispatch fs_backing_pdp10 d6fs d6fs_provider d6fs_validate_pdp10 d6fs_pdp10
MRES_ASM = $(KCORE_BUILD)/cty_io.s $(KCORE_BUILD)/clk_io.s $(KCORE_BUILD)/ptr_io.s $(KCORE_BUILD)/ptp_io.s $(KCORE_BUILD)/cr_io.s $(KCORE_BUILD)/cp_io.s $(KCORE_BUILD)/dcs_io.s $(KCORE_BUILD)/ge_io.s $(KCORE_BUILD)/dpy_io.s $(KCORE_BUILD)/tty_io.s $(KCORE_BUILD)/wcnsls_io.s $(KCORE_BUILD)/ocnsls_io.s $(KCORE_BUILD)/dsk_io.s $(KCORE_BUILD)/drm236_io.s $(KCORE_BUILD)/tape_io.s $(KCORE_BUILD)/slv_io.s $(KCORE_BUILD)/memfs_pdp10.s $(KCORE_BUILD)/dtfs.s $(KCORE_BUILD)/dtfs_pdp10.s $(KCORE_BUILD)/tsfs.s $(KCORE_BUILD)/tsfs_pdp10.s $(KCORE_BUILD)/blockset_dispatch.s $(KCORE_BUILD)/badmap_dispatch.s $(KCORE_BUILD)/fs_backing_pdp10.s $(KCORE_BUILD)/logstore_runtime.s $(KCORE_BUILD)/d6fs.s $(KCORE_BUILD)/d6fs_provider.s $(KCORE_BUILD)/d6fs_validate_pdp10.s $(KCORE_BUILD)/d6fs_pdp10.s

KCORE_DXR = $(BUILD)/kcore.dxr
KCORE_MAP = $(BUILD)/kcore.map
KCORE_FOLD_PLAN = $(BUILD)/kcore.fold
# Pre-self-hosting fold scope: all handwritten KCORE assembler.  Generated
# KCC output remains deliberately excluded until self-hosting/code-generator
# canonicalization work.  P10FOLD selects only relocation-safe zero-cost donors.
KCORE_FOLD_OBJS = $(BUILD)/ret.dobj \
	$(BUILD)/dsk270.dobj $(BUILD)/drm236.dobj \
	$(BUILD)/kfmt_pdp6.dobj $(BUILD)/vfs_pdp10.dobj \
	$(BUILD)/fs_mres_pdp10.dobj $(BUILD)/file_pdp10.dobj \
	$(BUILD)/pipe_pdp10.dobj $(BUILD)/monitorfs_pdp10.dobj \
	$(BUILD)/proc_pdp10.dobj $(BUILD)/proc_run_pdp10.dobj \
	$(BUILD)/device_state.dobj $(BUILD)/syscall_info_pdp10.dobj \
	$(BUILD)/pclk.dobj $(BUILD)/lpt_io.dobj \
	$(BUILD)/syscall_dispatch.dobj $(BUILD)/mach_user.dobj \
	$(BUILD)/kcore_io_pdp6.dobj $(BUILD)/storage_router.dobj $(BUILD)/kcore_end.dobj
KCORE_RIM = $(BUILD)/kcore.rim
KCORE_STREAM = $(BUILD)/kcore.stream
KCORE_EMBED_ASM = $(BUILD)/kcore-embed.s
KCORE_EMBED_OBJ = $(BUILD)/kcore-embed.dobj

CTY_MRES_OBJ = $(BUILD)/cty-mres.dobj
CLK_MRES_OBJ = $(BUILD)/clk-mres.dobj
PTR_MRES_OBJ = $(BUILD)/ptr-mres.dobj
PTP_MRES_OBJ = $(BUILD)/ptp-mres.dobj
CR_MRES_OBJ = $(BUILD)/cr-mres.dobj
CP_MRES_OBJ = $(BUILD)/cp-mres.dobj
DCS_MRES_OBJ = $(BUILD)/dcs-mres.dobj
GE_MRES_OBJ = $(BUILD)/ge-mres.dobj
DPY_MRES_OBJ = $(BUILD)/dpy-mres.dobj
TTY_MRES_OBJ = $(BUILD)/tty-mres.dobj
WCNSLS_MRES_OBJ = $(BUILD)/wcnsls-mres.dobj
OCNSLS_MRES_OBJ = $(BUILD)/ocnsls-mres.dobj
DSK_MRES_OBJ = $(BUILD)/dsk-mres.dobj
DRM_MRES_OBJ = $(BUILD)/drm-mres.dobj
TAPE_MRES_OBJ = $(BUILD)/tape-mres.dobj
SLV_MRES_OBJ = $(BUILD)/slv-mres.dobj
MEMFS_MRES_OBJ = $(BUILD)/memfs-mres.dobj
DTFS_MRES_OBJ = $(BUILD)/dtfs-mres.dobj
BLOCKSET_MRES_OBJ = $(BUILD)/blockset-mres.dobj
BADMAP_MRES_OBJ = $(BUILD)/badmap-mres.dobj
LOGSTORE_MRES_OBJ = $(BUILD)/logstore-mres.dobj
D6FS_MRES_OBJ = $(BUILD)/d6fs-mres.dobj
ifeq ($(BOOT_PROFILE),lowmem)
MRES_PACKAGE_OBJS = $(CTY_MRES_OBJ) $(CLK_MRES_OBJ) $(DCS_MRES_OBJ) \
	$(GE_MRES_OBJ) $(TTY_MRES_OBJ) $(DSK_MRES_OBJ) $(BLOCKSET_MRES_OBJ) \
	$(D6FS_MRES_OBJ)
KINIT_FULL = 0
else
MRES_PACKAGE_OBJS = $(CTY_MRES_OBJ) $(CLK_MRES_OBJ) $(PTR_MRES_OBJ) \
	$(PTP_MRES_OBJ) $(CR_MRES_OBJ) $(CP_MRES_OBJ) \
	$(DCS_MRES_OBJ) $(GE_MRES_OBJ) $(DPY_MRES_OBJ) $(TTY_MRES_OBJ) \
	$(WCNSLS_MRES_OBJ) $(OCNSLS_MRES_OBJ) $(TAPE_MRES_OBJ) $(DSK_MRES_OBJ) $(DRM_MRES_OBJ) \
	$(BLOCKSET_MRES_OBJ) $(LOGSTORE_MRES_OBJ) $(BADMAP_MRES_OBJ) $(MEMFS_MRES_OBJ) $(DTFS_MRES_OBJ) $(SLV_MRES_OBJ) \
	$(D6FS_MRES_OBJ)
KINIT_FULL = 1
endif

MINIT_TABLE_ASM = $(BUILD)/module-table-runtime.s
MINIT_TABLE_OBJ = $(BUILD)/module-table-runtime.dobj
END_OBJ = $(BUILD)/image-end.dobj
IMAGE_DXR = $(BUILD)/kinit.dxr
IMAGE_MAP = $(BUILD)/kinit.map
IMAGE_RIM = $(BUILD)/kinit.rim
IMAGE_RAW = $(BUILD)/kinit.raw.words
IMAGE_WORDS = $(BUILD)/kinit.words
IMAGE_LOWMEM_WORDS = $(BUILD)/kinit.lowmem.words
IMAGE_COMPRESSED_WORDS = $(BUILD)/kinit.d6lz.words
ifeq ($(BOOT_PROFILE),lowmem)
IMAGE_BOOT_WORDS = $(IMAGE_LOWMEM_WORDS)
else
IMAGE_BOOT_WORDS = $(IMAGE_COMPRESSED_WORDS)
endif

USERLAND_BUILD_ROOT ?= $(BUILD)/userland-build
INIT_DXR = $(USERLAND_BUILD_ROOT)/userland/init.dxr
MOUNTALL_DXR = $(USERLAND_BUILD_ROOT)/userland/mountall.dxr
LOGIN_DXR = $(USERLAND_BUILD_ROOT)/userland/login.dxr
DSH_DXR = $(USERLAND_BUILD_ROOT)/userland/dsh.dxr
LOGIN_D6LZ_DXR = $(USERLAND_BUILD_ROOT)/userland/login.d6lz.dxr
DSH_D6LZ_DXR = $(USERLAND_BUILD_ROOT)/userland/dsh.d6lz.dxr
TSFSPROBE_DXR = $(USERLAND_BUILD_ROOT)/userland/tsfsprobe.dxr
MOUNTTSFS_DXR = $(USERLAND_BUILD_ROOT)/userland/mounttsfs.dxr
DTFSPROBE_DXR = $(USERLAND_BUILD_ROOT)/userland/dtfsprobe.dxr
D6LZ_DXR = $(USERLAND_BUILD_ROOT)/userland/d6lz.dxr
LOGDRAIN_DXR = $(USERLAND_BUILD_ROOT)/userland/logdrain.dxr
SYSTEM_INIT_DXR ?= $(INIT_DXR)
SYSTEM_MOUNTALL_DXR ?= $(MOUNTALL_DXR)
SYSTEM_LOGIN_DXR ?= $(LOGIN_D6LZ_DXR)
SYSTEM_DSH_DXR ?= $(DSH_D6LZ_DXR)
SYSTEM_TSFSPROBE_DXR ?= $(TSFSPROBE_DXR)
SYSTEM_MOUNTTSFS_DXR ?= $(MOUNTTSFS_DXR)
SYSTEM_DTFSPROBE_DXR ?= $(DTFSPROBE_DXR)
SYSTEM_D6LZ_DXR ?= $(D6LZ_DXR)
SYSTEM_LOGDRAIN_DXR ?= $(LOGDRAIN_DXR)
SYSTEM_INITTAB_TEXT ?= $(PROJECT_ROOT)/userland/config/inittab
SYSTEM_PASSWD_TEXT ?= $(PROJECT_ROOT)/userland/config/passwd
ifeq ($(BOOT_PROFILE),lowmem)
SYSTEM_FSTAB_TEXT ?= $(PROJECT_ROOT)/userland/config/fstab.lowmem
else
SYSTEM_FSTAB_TEXT ?= $(PROJECT_ROOT)/userland/config/fstab
endif
SYSTEM_CONFIG_BUILD = $(BUILD)/config
SYSTEM_INITTAB_S6REC = $(SYSTEM_CONFIG_BUILD)/INITTAB.s6rec
SYSTEM_PASSWD_S6REC = $(SYSTEM_CONFIG_BUILD)/PASSWD.s6rec
SYSTEM_FSTAB_S6REC = $(SYSTEM_CONFIG_BUILD)/FSTAB.s6rec
SYSTEM_CONFIG_S6REC = $(SYSTEM_INITTAB_S6REC) $(SYSTEM_PASSWD_S6REC) \
	$(SYSTEM_FSTAB_S6REC)
MANUAL_BUILD = $(BUILD)/manual
SYSTEM_INIT_MANUAL_SRC = $(PROJECT_ROOT)/userland/manual/INIT.SIXMD
SYSTEM_LOGIN_MANUAL_SRC = $(PROJECT_ROOT)/userland/manual/LOGIN.SIXMD
SYSTEM_TSFSPROBE_MANUAL_SRC = $(PROJECT_ROOT)/userland/manual/TSFSPROBE.SIXMD
SYSTEM_MOUNTTSFS_MANUAL_SRC = $(PROJECT_ROOT)/userland/manual/MOUNT.TSFS.SIXMD
SYSTEM_D6LZ_MANUAL_SRC = $(PROJECT_ROOT)/userland/manual/D6LZ.SIXMD
SYSTEM_LOGDRAIN_MANUAL_SRC = $(PROJECT_ROOT)/userland/manual/LOGDRAIN.SIXMD
SYSTEM_INIT_MANUAL = $(MANUAL_BUILD)/INIT.s6rec
SYSTEM_LOGIN_MANUAL = $(MANUAL_BUILD)/LOGIN.s6rec
SYSTEM_TSFSPROBE_MANUAL = $(MANUAL_BUILD)/TSFSPROBE.s6rec
SYSTEM_MOUNTTSFS_MANUAL = $(MANUAL_BUILD)/MOUNT.TSFS.s6rec
SYSTEM_D6LZ_MANUAL = $(MANUAL_BUILD)/D6LZ.s6rec
SYSTEM_LOGDRAIN_MANUAL = $(MANUAL_BUILD)/LOGDRAIN.s6rec
SYSTEM_MANUALS = $(SYSTEM_INIT_MANUAL) $(SYSTEM_LOGIN_MANUAL) \
	$(SYSTEM_TSFSPROBE_MANUAL) $(SYSTEM_MOUNTTSFS_MANUAL) $(SYSTEM_D6LZ_MANUAL) \
	$(SYSTEM_LOGDRAIN_MANUAL)
USERLAND_INPUTS = $(PROJECT_ROOT)/userland/libc/Makefile \
	$(PROJECT_ROOT)/userland/libc/crt0.s \
	$(PROJECT_ROOT)/userland/libc/dsys.h \
	$(PROJECT_ROOT)/userland/libc/memcpy.s \
	$(PROJECT_ROOT)/userland/libc/memmove.s \
	$(PROJECT_ROOT)/userland/libc/syscall.s \
	$(PROJECT_ROOT)/userland/libc/syscall_helpers.s \
	$(PROJECT_ROOT)/userland/libc/u.c \
	$(PROJECT_ROOT)/userland/libc/u.h \
	$(PROJECT_ROOT)/userland/libc/text.c \
	$(PROJECT_ROOT)/userland/libc/text.h \
	$(PROJECT_ROOT)/userland/logstore/logdrain.c \
	$(PROJECT_ROOT)/userland/exec/COMMANDS \
	$(PROJECT_ROOT)/userland/exec/cmd.h \
	$(PROJECT_ROOT)/userland/exec/commands.c \
	$(PROJECT_ROOT)/userland/exec/standalone.c \
	$(PROJECT_ROOT)/userland/dsh/dsh.c \
	$(PROJECT_ROOT)/userland/d6lz/d6lz.c \
	$(PROJECT_ROOT)/userland/init/init.c \
	$(PROJECT_ROOT)/userland/login/login.c \
	$(PROJECT_ROOT)/userland/tsfs/tsfs_scan.h \
	$(PROJECT_ROOT)/userland/tsfs/tsfs_scan.c \
	$(PROJECT_ROOT)/userland/tsfs/tsfsprobe.c \
	$(PROJECT_ROOT)/userland/tsfs/mount_tsfs.c \
	$(PROJECT_ROOT)/userland/dtfs/dtfs_probe.h \
	$(PROJECT_ROOT)/userland/dtfs/dtfs_probe.c \
	$(PROJECT_ROOT)/userland/dtfs/dtfsprobe.c \
	$(PROJECT_ROOT)/userland/config/inittab \
	$(PROJECT_ROOT)/userland/config/passwd \
	$(PROJECT_ROOT)/userland/Makefile \
	$(PROJECT_ROOT)/system/kernel/proc/syscall.h \
	$(PROJECT_ROOT)/system/kernel/fs/dtfs_media.h

ifeq ($(BOOT),auto)
ifeq ($(ROOT),drum)
STAGE1_DEVICE = drm
else
STAGE1_DEVICE = dsk
endif
else
STAGE1_DEVICE = $(BOOT)
endif
ifneq ($(filter $(STAGE1_DEVICE),dsk drm dtc mtc ptr),$(STAGE1_DEVICE))
$(error unsupported BOOT=$(BOOT); expected auto, dsk, drm, dtc, mtc, or ptr)
endif
STAGE1_SRC = $(PROJECT_ROOT)/system/stand/pdp6/$(STAGE1_DEVICE)/stage1.s
STAGE1_COMMON = $(PROJECT_ROOT)/system/stand/pdp6/common/decompressor.inc \
	$(PROJECT_ROOT)/system/stand/pdp6/common/stage1-error.inc \
	$(PROJECT_ROOT)/system/stand/pdp6/common/sixbit-fixed.inc
STAGE1_DXR = $(BUILD)/stage1-$(STAGE1_DEVICE).dxr
STAGE1_TAPE = $(BUILD)/stage1-$(STAGE1_DEVICE).pt
DISK_DIR = $(BUILD)/disk
DISK_IMAGE = $(DISK_DIR)/dsk0.dsk
SIMH_INI = $(BUILD)/boot.ini
SIMH_LOG = $(BUILD)/boot.simh.log
MEDIA_DIR = $(BUILD)/media
DTC0_IMAGE = $(MEDIA_DIR)/dtc0.tap
DTC1_IMAGE = $(MEDIA_DIR)/dtc1.tap
DTC2_IMAGE = $(MEDIA_DIR)/dtc2.tap
TSFS_ROOT_MANIFEST = $(BUILD)/tsfs-root.manifest
TSFS_ROOT_PREFIX = $(MEDIA_DIR)/tsfs-root
TSFS_ROOT_IMAGE = $(TSFS_ROOT_PREFIX)0.dta
MTC0_IMAGE = $(MEDIA_DIR)/mtc0.tap
MTC1_IMAGE = $(MEDIA_DIR)/mtc1.tap
DR0_IMAGE = $(MEDIA_DIR)/dr0.drm
DR1_IMAGE = $(MEDIA_DIR)/dr1.drm
DR2_IMAGE = $(MEDIA_DIR)/dr2.drm
DR3_IMAGE = $(MEDIA_DIR)/dr3.drm

ifeq ($(ROOT),tape)
ROOT_MEDIA_DEPS = $(TSFS_ROOT_IMAGE)
else
ROOT_MEDIA_DEPS =
endif
PTP_OUTPUT = $(MEDIA_DIR)/ptp.out
CP_OUTPUT = $(MEDIA_DIR)/cp.out
LPT_OUTPUT = $(MEDIA_DIR)/lpt.out
BOOT_MEDIA_STAMP = $(BUILD)/.boot-media-$(STAGE1_DEVICE)
BOOT_COMMANDS = $(BUILD)/boot-commands-$(STAGE1_DEVICE).simh
PTR_BOOT1_WORDS = $(BUILD)/ptr-boot1.words
PTR_BOOT2_WORDS = $(BUILD)/ptr-boot2.words
PTR_BOOT1 = $(MEDIA_DIR)/ptr-boot1.pt
PTR_BOOT2 = $(MEDIA_DIR)/ptr-boot2.pt
SIMH_DCS0_PORT ?= 1101
SIMH_GE0_PORT ?= 1201
SIMH_PCLK_MODE ?= REALTIME

.PHONY: image boot permanent-size clean force-simh-config

force-simh-config:

$(BUILD):
	mkdir -p $@

$(MEDIA_DIR): | $(BUILD)
	mkdir -p $@

$(KINIT_STAMP): $(KERNEL_INPUTS) $(BUILD)
	$(MAKE) -C $(KERNEL_DIR) kinit BUILD='$(KINIT_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' KINIT_STACK_RESERVE_WORDS='$(KINIT_STACK_RESERVE_WORDS)' KINIT_STACK_WATERMARK='$(KINIT_STACK_WATERMARK)' KINIT_BADMAP='$(KINIT_BADMAP)' KINIT_FULL='$(KINIT_FULL)' PROC_STACK_WATERMARK='$(PROC_STACK_WATERMARK)' VM_BACKEND='$(VM_BACKEND)'
	touch $@

$(KCORE_STAMP): $(KERNEL_INPUTS) $(BUILD)
	$(MAKE) -C $(KERNEL_DIR) kcore BUILD='$(KCORE_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' KINIT_STACK_WATERMARK='$(KINIT_STACK_WATERMARK)' PROC_BOOT_USERS='$(PROC_BOOT_USERS)' PROC_STACK_WATERMARK='$(PROC_STACK_WATERMARK)' VM_BACKEND='$(VM_BACKEND)'
	touch $@

$(KINIT_ASM) $(KINIT_LATE_ASM): $(KINIT_STAMP)
	@test -f $@

$(KCORE_FIXED_ASM) $(MRES_ASM): $(KCORE_STAMP)
	@test -f $@

KERNEL_ASM_OBJS = $(BUILD)/kinit.dobj $(BUILD)/kinit_diag.dobj $(BUILD)/module_minit.dobj $(BUILD)/kinit_boot.dobj $(BUILD)/root_select.dobj $(BUILD)/tsfs_boot.dobj $(BUILD)/d6fs_boot.dobj $(BUILD)/d6fs_super_boot.dobj $(BUILD)/blockset_boot.dobj $(BUILD)/logstore_boot.dobj $(BUILD)/logstore.dobj $(BUILD)/proc_boot.dobj $(BUILD)/proc_swap_boot.dobj $(BUILD)/kinit_io_pdp6.dobj $(BUILD)/module_io.dobj $(BUILD)/bootbanner_generated.dobj $(BUILD)/kinit_late_begin_pdp6.dobj $(BUILD)/proc_late.dobj $(BUILD)/mm_boot.dobj $(BUILD)/kinit_late.dobj $(BUILD)/kinit_late_end.dobj $(BUILD)/d6lz_fixed.dobj $(BUILD)/exec.dobj $(BUILD)/d6lz.dobj $(BUILD)/mm.dobj $(BUILD)/proc.dobj $(BUILD)/vm_backend.dobj $(BUILD)/proc_swap.dobj $(BUILD)/file.dobj $(BUILD)/ret.dobj $(BUILD)/dsk270.dobj $(BUILD)/drm236.dobj $(BUILD)/kfmt_pdp6.dobj $(BUILD)/vfs_pdp10.dobj $(BUILD)/fs_mres_pdp10.dobj $(BUILD)/file_pdp10.dobj $(BUILD)/pipe_pdp10.dobj $(BUILD)/monitorfs_pdp10.dobj $(BUILD)/proc_pdp10.dobj $(BUILD)/proc_run_pdp10.dobj $(BUILD)/device_state.dobj $(BUILD)/syscall_info_pdp10.dobj $(BUILD)/pclk.dobj $(BUILD)/syscall_dispatch.dobj $(BUILD)/mach_user.dobj $(BUILD)/vm_backend_asm.dobj $(BUILD)/kcore_io_pdp6.dobj $(BUILD)/storage_router.dobj $(BUILD)/kcore_end.dobj $(BUILD)/cty_io.dobj $(BUILD)/clk_io.dobj $(BUILD)/ptr_io.dobj $(BUILD)/ptp_io.dobj $(BUILD)/lpt_io.dobj $(BUILD)/cr_io.dobj $(BUILD)/cp_io.dobj $(BUILD)/dcs_io.dobj $(BUILD)/ge_io.dobj $(BUILD)/dpy_io.dobj $(BUILD)/tty_io.dobj $(BUILD)/wcnsls_io.dobj $(BUILD)/ocnsls_io.dobj $(BUILD)/dsk_io.dobj $(BUILD)/drm236_io.dobj $(BUILD)/tape_io.dobj $(BUILD)/slv_io.dobj $(BUILD)/memfs_pdp10.dobj $(BUILD)/dtfs.dobj $(BUILD)/dtfs_pdp10.dobj $(BUILD)/tsfs.dobj $(BUILD)/tsfs_pdp10.dobj $(BUILD)/blockset_dispatch.dobj $(BUILD)/badmap_dispatch.dobj $(BUILD)/fs_backing_pdp10.dobj $(BUILD)/logstore_runtime.dobj $(BUILD)/d6fs.dobj $(BUILD)/d6fs_provider.dobj $(BUILD)/d6fs_validate_pdp10.dobj $(BUILD)/d6fs_pdp10.dobj

$(KERNEL_ASM_OBJS): $(KINIT_ASM) $(KINIT_LATE_ASM) $(KCORE_FIXED_ASM) $(MRES_ASM)
	@mkdir -p $(BUILD)
	@base=`basename "$@" .dobj`; src=; if test -f "$(KINIT_BUILD)/$$base.s"; then src="$(KINIT_BUILD)/$$base.s"; elif test -f "$(KCORE_BUILD)/$$base.s"; then src="$(KCORE_BUILD)/$$base.s"; fi; test -n "$$src"; $(DAS) $(DASFLAGS) -c -o "$@" "$$src"

force-minit-config:

$(MINIT_TABLE_ASM): $(KERNEL_DIR)/modules/module_table.s force-minit-config | $(BUILD)
	{ echo ".set KINIT_FULL,$(KINIT_FULL)"; echo ".set KINIT_BADMAP,$(KINIT_BADMAP)"; cat $<; } > $@

$(MINIT_TABLE_OBJ): $(MINIT_TABLE_ASM) $(BUILD)
	$(DAS) $(DASFLAGS) -c -o $@ $<

$(END_OBJ): image/image-end.s $(BUILD)
	$(DAS) $(DASFLAGS) -c -o $@ $<

$(KCORE_FOLD_PLAN): $(KCORE_FOLD_OBJS) $(P10FOLD)
	$(P10FOLD) -m 4 -P $@ $(KCORE_FOLD_OBJS) >/dev/null

$(KCORE_DXR): $(KCORE_FIXED_OBJS) $(KCORE_FOLD_PLAN)
	libgcc=`$(PDP10_GCC) -print-libgcc-file-name`; \
	$(DLINK) -b 060 --fold-plan $(KCORE_FOLD_PLAN) -o $@ -M $(KCORE_MAP) \
	    $(KCORE_FIXED_OBJS) $$libgcc

$(KCORE_MAP): $(KCORE_DXR)
	@test -f $@

$(KCORE_RIM): $(KCORE_DXR)
	$(DXRCONVERT) --simh -b 060 $< $@

$(KCORE_STREAM): $(KCORE_RIM) $(KCORE_MAP)
	@end=; \
	while read name value rest; do \
		if test "$$name" = "__kcore_low_init_end"; then \
			end=$$value; \
			break; \
		fi; \
	done < $(KCORE_MAP); \
	test -n "$$end"; words=$$((0$$end - 060)); words=`printf '%o' "$$words"`; \
	$(MKSTREAM) -i $(KCORE_RIM) -o $@ -b 060 -w "$$words"

$(CTY_MRES_OBJ): $(BUILD)/cty_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cty-mres.dxr -M $(BUILD)/cty-mres.map \
	    -R $@ -N cty_mres_package -X cty_pi_handler -X cty_putchar \
	    -X cty_getchar $(BUILD)/cty_io.dobj

$(CLK_MRES_OBJ): $(BUILD)/clk_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/clk-mres.dxr -M $(BUILD)/clk-mres.map \
	    -R $@ -N clk_mres_package -X clk_pi_handler -X clk_ticks -X clk_pi_service \
	    $(BUILD)/clk_io.dobj

$(PTR_MRES_OBJ): $(BUILD)/ptr_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ptr-mres.dxr -M $(BUILD)/ptr-mres.map \
	    -R $@ -N ptr_mres_package -X ptr_pi_handler -X ptr_getchar \
	    $(BUILD)/ptr_io.dobj

$(PTP_MRES_OBJ): $(BUILD)/ptp_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ptp-mres.dxr -M $(BUILD)/ptp-mres.map \
	    -R $@ -N ptp_mres_package -X ptp_pi_handler -X ptp_putchar \
	    $(BUILD)/ptp_io.dobj

$(CR_MRES_OBJ): $(BUILD)/cr_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cr-mres.dxr -M $(BUILD)/cr-mres.map \
	    -R $@ -N cr_mres_package -X cr_pi_handler -X cr_read_card \
	    $(BUILD)/cr_io.dobj

$(CP_MRES_OBJ): $(BUILD)/cp_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cp-mres.dxr -M $(BUILD)/cp-mres.map \
	    -R $@ -N cp_mres_package -X cp_pi_handler -X cp_punch_card \
	    $(BUILD)/cp_io.dobj

$(DCS_MRES_OBJ): $(BUILD)/dcs_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/dcs-mres.dxr -M $(BUILD)/dcs-mres.map \
	    -R $@ -N dcs_mres_package -X dcs_pi_handler -X dcs_getchar -X dcs_putchar \
	    $(BUILD)/dcs_io.dobj

$(GE_MRES_OBJ): $(BUILD)/ge_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ge-mres.dxr -M $(BUILD)/ge-mres.map \
	    -R $@ -N ge_mres_package -X ge_pi_handler -X ge_getchar -X ge_putchar \
	    $(BUILD)/ge_io.dobj

$(DPY_MRES_OBJ): $(BUILD)/dpy_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/dpy-mres.dxr -M $(BUILD)/dpy-mres.map \
	    -R $@ -N dpy_mres_package -X dpy_pi_handler -X dpy_putword \
	    -X dpy_clk_pi_service_call $(BUILD)/dpy_io.dobj

$(TTY_MRES_OBJ): $(BUILD)/tty_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/tty-mres.dxr -M $(BUILD)/tty-mres.map \
	    -R $@ -N tty_mres_package -X tty_putchar -X tty_getchar -X tty_cty_putchar_address \
	    -X tty_dcs_putchar_address -X tty_ge_putchar_address \
	    -X tty_cty_getchar_address -X tty_dcs_getchar_address \
	    -X tty_ge_getchar_address $(BUILD)/tty_io.dobj

$(WCNSLS_MRES_OBJ): $(BUILD)/wcnsls_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/wcnsls-mres.dxr -M $(BUILD)/wcnsls-mres.map \
	    -R $@ -N wcnsls_mres_package -X wcnsls_read -X wcnsls_cono -X wcnsls_plot \
	    $(BUILD)/wcnsls_io.dobj

$(OCNSLS_MRES_OBJ): $(BUILD)/ocnsls_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ocnsls-mres.dxr -M $(BUILD)/ocnsls-mres.map \
	    -R $@ -N ocnsls_mres_package -X ocnsls_read $(BUILD)/ocnsls_io.dobj

$(DSK_MRES_OBJ): $(BUILD)/dsk_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/dsk-mres.dxr -M $(BUILD)/dsk-mres.map \
	    -R $@ -N dsk_mres_package -X dsk_pi_handler -X dsk_read_sector \
	    -X dsk_write_sector -X dsk_dct_handler -X dsk_watchdog_tick \
	    $(BUILD)/dsk_io.dobj

$(DRM_MRES_OBJ): $(BUILD)/drm236_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/drm-mres.dxr -M $(BUILD)/drm-mres.map \
	    -R $@ -N drm236_mres_package -X drm236_pi_handler \
	    -X drm236_read_block_service -X drm236_write_block_service \
	    $(BUILD)/drm236_io.dobj

$(TAPE_MRES_OBJ): $(BUILD)/tape_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/tape-mres.dxr -M $(BUILD)/tape-mres.map \
	    -R $@ -N tape_mres_package -X tape_pi_handler -X dtc_read_block \
	    -X mtc_service -X dtc_write_block -X tape_dct_handler $(BUILD)/tape_io.dobj

$(SLV_MRES_OBJ): $(BUILD)/slv_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/slv-mres.dxr -M $(BUILD)/slv-mres.map \
	    -R $@ -N slv_mres_package -X slv_pi_handler $(BUILD)/slv_io.dobj

$(MEMFS_MRES_OBJ): $(BUILD)/memfs_pdp10.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/memfs-mres.dxr -M $(BUILD)/memfs-mres.map \
	    -R $@ -N memfs_mres_package -X memfs_mres_dispatch -X memfs_mres_usage \
	    $(BUILD)/memfs_pdp10.dobj

$(DTFS_MRES_OBJ): $(BUILD)/dtfs.dobj $(BUILD)/dtfs_pdp10.dobj $(BUILD)/tsfs.dobj $(BUILD)/tsfs_pdp10.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/dtfs-mres.dxr -M $(BUILD)/dtfs-mres.map \
	    -R $@ -N dtfs_mres_package -X dtfs_mres_dispatch \
	    -X dtfs_dtc_read_jump -X dtfs_dtc_write_jump \
	    -X dtfs_mount_unit -X tsfs_mres_dispatch -X dtfs_dir \
	    $(BUILD)/dtfs.dobj $(BUILD)/dtfs_pdp10.dobj $(BUILD)/tsfs.dobj $(BUILD)/tsfs_pdp10.dobj

$(BLOCKSET_MRES_OBJ): $(BUILD)/blockset_dispatch.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/blockset-mres.dxr -M $(BUILD)/blockset-mres.map \
	    -R $@ -N blockset_mres_package -X blockset_map_block \
	    -X blockset_boot -X blockset_read_block -X blockset_write_block \
	    -X blockset_backend_read_jump -X blockset_backend_write_jump \
	    $(BUILD)/blockset_dispatch.dobj


$(BADMAP_MRES_OBJ): $(BUILD)/badmap_dispatch.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/badmap-mres.dxr -M $(BUILD)/badmap-mres.map \
	    -R $@ -N badmap_mres_package -X badmap_map_block -X badmap_state \
	    -X badmap_read_block -X badmap_write_block \
	    -X badmap_backend_read_jump -X badmap_backend_write_jump \
	    -X badmap_base_map_jump $(BUILD)/badmap_dispatch.dobj

$(LOGSTORE_MRES_OBJ): $(BUILD)/logstore_runtime.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/logstore-mres.dxr -M $(BUILD)/logstore-mres.map \
	    -R $@ -N logstore_mres_package -X logstore_mres_dispatch \
	    -X logstore_mres_state -X logstore_backend_read_jump \
	    -X logstore_backend_write_jump $(BUILD)/logstore_runtime.dobj

$(D6FS_MRES_OBJ): $(BUILD)/fs_backing_pdp10.dobj $(BUILD)/d6fs.dobj $(BUILD)/d6fs_provider.dobj $(BUILD)/d6fs_validate_pdp10.dobj $(BUILD)/d6fs_pdp10.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/d6fs-mres.dxr -M $(BUILD)/d6fs-mres.map \
	    -R $@ -N d6fs_mres_package -X d6fs_mres_dispatch \
	    -X fs_backing_root_read -X fs_backing_root_write \
	    -X fs_backing_root_read_jump -X fs_backing_root_write_jump \
	    -X d6fs_cache_reclaim \
	    -X fs_backing_direct_read -X fs_backing_direct_write \
	    -X fs_backing_direct_read_jump -X fs_backing_direct_write_jump \
	    -X fs_backing_direct_drm_read_jump -X fs_backing_direct_drm_write_jump \
	    $(BUILD)/fs_backing_pdp10.dobj $(BUILD)/d6fs.dobj $(BUILD)/d6fs_provider.dobj $(BUILD)/d6fs_validate_pdp10.dobj $(BUILD)/d6fs_pdp10.dobj

# KINIT contains one load slot whose size must exactly match initialized KCORE.
# Read the absolute end symbol using shell built-ins only, then emit the tiny
# assembler reservation directly.  This deliberately avoids AWK/sed so the
# step remains suitable for a small future native DAIMOS build environment.
$(KCORE_EMBED_ASM): $(KCORE_MAP) $(BUILD)
	@end=; \
	while read name value rest; do \
		if test "$$name" = "__kcore_low_init_end"; then \
			end=$$value; \
			break; \
		fi; \
	done < $(KCORE_MAP); \
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

$(KCORE_EMBED_OBJ): $(KCORE_EMBED_ASM)
	$(DAS) $(DASFLAGS) -c -o $@ $<

$(SYSTEM_CONFIG_BUILD):
	mkdir -p $@

$(SYSTEM_INITTAB_S6REC): $(SYSTEM_INITTAB_TEXT) | $(SYSTEM_CONFIG_BUILD)
	$(S6TEXT) --encode $< $@

$(SYSTEM_FSTAB_S6REC): $(SYSTEM_FSTAB_TEXT) | $(SYSTEM_CONFIG_BUILD)
	$(S6TEXT) --encode $< $@

$(SYSTEM_PASSWD_S6REC): $(SYSTEM_PASSWD_TEXT) | $(SYSTEM_CONFIG_BUILD)
	$(S6TEXT) --encode $< $@

$(MANUAL_BUILD):
	mkdir -p $@

$(SYSTEM_INIT_MANUAL): $(SYSTEM_INIT_MANUAL_SRC) | $(MANUAL_BUILD)
	$(S6FILTER) $< > $@

$(SYSTEM_LOGIN_MANUAL): $(SYSTEM_LOGIN_MANUAL_SRC) | $(MANUAL_BUILD)
	$(S6FILTER) $< > $@

$(SYSTEM_TSFSPROBE_MANUAL): $(SYSTEM_TSFSPROBE_MANUAL_SRC) | $(MANUAL_BUILD)
	$(S6FILTER) $< > $@

$(SYSTEM_MOUNTTSFS_MANUAL): $(SYSTEM_MOUNTTSFS_MANUAL_SRC) | $(MANUAL_BUILD)
	$(S6FILTER) $< > $@

$(SYSTEM_D6LZ_MANUAL): $(SYSTEM_D6LZ_MANUAL_SRC) | $(MANUAL_BUILD)
	$(S6FILTER) $< > $@

$(SYSTEM_LOGDRAIN_MANUAL): $(SYSTEM_LOGDRAIN_MANUAL_SRC) | $(MANUAL_BUILD)
	$(S6FILTER) $< > $@

$(INIT_DXR) $(MOUNTALL_DXR) $(LOGIN_DXR) $(DSH_DXR) $(TSFSPROBE_DXR) $(MOUNTTSFS_DXR) $(DTFSPROBE_DXR) $(D6LZ_DXR) $(LOGDRAIN_DXR) &: $(USERLAND_INPUTS)
	$(MAKE) -C $(PROJECT_ROOT)/userland build BUILD_ROOT='$(USERLAND_BUILD_ROOT)' \
	    PDP10_PREFIX='$(PDP10_PREFIX)'

$(LOGIN_D6LZ_DXR): $(LOGIN_DXR)
	$(D6LZ_HOST) -x $< $@

$(DSH_D6LZ_DXR): $(DSH_DXR)
	$(D6LZ_HOST) -x $< $@

$(IMAGE_DXR): $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
    $(MRES_PACKAGE_OBJS) $(KINIT_LATE_OBJS) $(END_OBJ) $(KCORE_MAP)
	$(DLINK) -b 030000 -A $(KCORE_MAP) -o $@ -M $(IMAGE_MAP) \
	    $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
	    $(MRES_PACKAGE_OBJS) $(KINIT_LATE_OBJS) $(END_OBJ)

$(IMAGE_MAP): $(IMAGE_DXR)
	@test -f $@

$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 030000 $< $@

$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	@end=; \
	while read name value rest; do \
		if test "$$name" = "__kinit_image_end"; then \
			end=$$value; \
			break; \
		fi; \
	done < $(IMAGE_MAP); \
	test -n "$$end"; words=$$((0$$end - 030000)); words=`printf '%o' "$$words"`; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 030000 -w "$$words"

$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(KCORE_STREAM) $(KCORE_MAP) image/mkimage.sh
	./image/mkimage.sh --map $(IMAGE_MAP) --input $(IMAGE_RAW) \
	    --kcore $(KCORE_STREAM) --kcore-map $(KCORE_MAP) \
	    --core-words $(BOOT_CORE_WORDS) \
	    --stack-words $(KINIT_STACK_RESERVE_WORDS) --output $@

$(IMAGE_LOWMEM_WORDS): $(IMAGE_WORDS)
	@set -eu; \
	tmp='$@.tmp.$$$$'; \
	trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	{ head -n 2 $(IMAGE_WORDS); printf '%012o\n' 0; tail -n +3 $(IMAGE_WORDS); } > "$$tmp"; \
	mv "$$tmp" $@; \
	trap - 0 1 2 3 15

$(IMAGE_COMPRESSED_WORDS): $(IMAGE_WORDS)
	@set -eu; \
	tmp_raw='$@.raw.$$$$'; tmp_comp='$@.comp.$$$$'; tmp_out='$@.tmp.$$$$'; \
	trap 'rm -f "$$tmp_raw" "$$tmp_comp" "$$tmp_out"' 0 1 2 3 15; \
	tail -n +3 $(IMAGE_WORDS) > "$$tmp_raw"; \
	$(D6LZ_HOST) -t "$$tmp_raw" "$$tmp_comp"; \
	cwords=$$(wc -l < "$$tmp_comp" | tr -d ' '); \
	desc=$$(sed -n '2p' $(IMAGE_WORDS)); \
	uwords=$$((0$${desc%??????})); \
	stage_end=$$((030000 + uwords + cwords)); \
	core_words=$$((0$(BOOT_CORE_WORDS))); \
	if [ $$stage_end -gt $$core_words ]; then \
		echo "compressed boot image exceeds configured core: stage_end=$$(printf '%o' $$stage_end) core=$$(printf '%o' $$core_words)" >&2; \
		exit 1; \
	fi; \
	{ head -n 2 $(IMAGE_WORDS); printf '%06o%06o\n' $$cwords 0; cat "$$tmp_comp"; } > "$$tmp_out"; \
	mv "$$tmp_out" $@; \
	trap - 0 1 2 3 15

$(STAGE1_DXR): $(STAGE1_SRC) $(STAGE1_COMMON) $(BUILD)
	$(DAS) $(DASFLAGS) -k -o $@ $<

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b 060 $< $@
	printf '\252' >> $@

$(DISK_IMAGE): $(IMAGE_BOOT_WORDS) $(SYSTEM_INIT_DXR) $(SYSTEM_MOUNTALL_DXR) $(SYSTEM_LOGIN_DXR) $(SYSTEM_DSH_DXR) $(SYSTEM_TSFSPROBE_DXR) $(SYSTEM_MOUNTTSFS_DXR) $(SYSTEM_DTFSPROBE_DXR) $(SYSTEM_D6LZ_DXR) $(SYSTEM_LOGDRAIN_DXR) $(SYSTEM_CONFIG_S6REC) $(SYSTEM_MANUALS)
	rm -rf $(DISK_DIR)
	$(MKDSK) -n 1 -m clean -p $(IMAGE_BOOT_WORDS) -o $(DISK_DIR) \
	    --d6fs-layout --logstore-blocks $(D6FS_LOGSTORE_BLOCKS) \
	    --swap-tail-blocks $(D6FS_SWAP_TAIL_BLOCKS) \
	    --spare-blocks $(D6FS_SPARE_BLOCKS)
	$(MKD6FS) -n 1 -d $(DISK_DIR) \
	    -D /TEMP:0777 \
	    -D /MOUNT/DT0:0755 -D /MOUNT/DT1:0755 -D /MOUNT/DT2:0755 \
	    -D /SYSTEM:0555 -D /SYSTEM/EXEC:0555 -D /SYSTEM/LIB:0555 \
	    -D /SYSTEM/INCLUDE:0555 -D /SYSTEM/MANUAL:0555 -D /SYSTEM/SOURCE:0555 \
	    -D /OPTION:0555 -D /OPTION/EXEC:0555 -D /OPTION/LIB:0555 \
	    -D /OPTION/INCLUDE:0555 -D /OPTION/MANUAL:0555 -D /OPTION/SOURCE:0555 \
	    -D /CONFIG:0755 \
	    -f "/SYSTEM/INIT:$(SYSTEM_INIT_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/MOUNTALL:$(SYSTEM_MOUNTALL_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/LOGIN:$(SYSTEM_LOGIN_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/DSH:$(SYSTEM_DSH_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/TSFSPROBE:$(SYSTEM_TSFSPROBE_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/MOUNT.TSFS:$(SYSTEM_MOUNTTSFS_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/DTFSPROBE:$(SYSTEM_DTFSPROBE_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/D6LZ:$(SYSTEM_D6LZ_DXR):555:dxr" \
	    -f "/SYSTEM/EXEC/LOGDRAIN:$(SYSTEM_LOGDRAIN_DXR):555:dxr" \
	    -f "/SYSTEM/MANUAL/INIT:$(SYSTEM_INIT_MANUAL):444:binwords" \
	    -f "/SYSTEM/MANUAL/LOGIN:$(SYSTEM_LOGIN_MANUAL):444:binwords" \
	    -f "/SYSTEM/MANUAL/TSFSPROBE:$(SYSTEM_TSFSPROBE_MANUAL):444:binwords" \
	    -f "/SYSTEM/MANUAL/MOUNT.TSFS:$(SYSTEM_MOUNTTSFS_MANUAL):444:binwords" \
	    -f "/SYSTEM/MANUAL/D6LZ:$(SYSTEM_D6LZ_MANUAL):444:binwords" \
	    -f "/SYSTEM/MANUAL/LOGDRAIN:$(SYSTEM_LOGDRAIN_MANUAL):444:binwords" \
	    -f "/CONFIG/INITTAB:$(SYSTEM_INITTAB_S6REC):644:binwords" \
	    -f "/CONFIG/FSTAB:$(SYSTEM_FSTAB_S6REC):644:binwords" \
	    -f "/CONFIG/PASSWD:$(SYSTEM_PASSWD_S6REC):600:binwords"
	$(LOGSTORE) -n 1 -d $(DISK_DIR) --init

$(TSFS_ROOT_IMAGE): $(INIT_DXR) $(SYSTEM_MOUNTALL_DXR) $(LOGIN_DXR) $(DSH_DXR) $(DTFSPROBE_DXR) \
	$(SYSTEM_CONFIG_S6REC) | $(MEDIA_DIR)
	@set -eu; \
	tmp='$(TSFS_ROOT_MANIFEST).tmp.$$$$'; \
	trap 'rm -f "$$tmp"' 0 1 2 3 15; \
	{ \
	  echo 'D /MOUNT'; \
	  echo 'D /TEMP'; \
	  echo 'D /SYSTEM'; \
	  echo 'D /SYSTEM/EXEC'; \
	  echo 'D /CONFIG'; \
	  printf 'F /SYSTEM/INIT %s\n' '$(INIT_DXR)'; \
	  printf 'F /SYSTEM/EXEC/MOUNTALL %s\n' '$(SYSTEM_MOUNTALL_DXR)'; \
	  printf 'F /CONFIG/FSTAB %s\n' '$(SYSTEM_FSTAB_S6REC)'; \
	  printf 'F /CONFIG/INITTAB %s\n' '$(SYSTEM_INITTAB_S6REC)'; \
	  printf 'F /SYSTEM/EXEC/LOGIN %s\n' '$(LOGIN_DXR)'; \
	  printf 'F /CONFIG/PASSWD %s\n' '$(SYSTEM_PASSWD_S6REC)'; \
	  printf 'F /SYSTEM/EXEC/DSH %s\n' '$(DSH_DXR)'; \
	  printf 'F /SYSTEM/EXEC/DTFSPROBE %s\n' '$(DTFSPROBE_DXR)'; \
	} > "$$tmp"; \
	mv "$$tmp" '$(TSFS_ROOT_MANIFEST)'; \
	trap - 0 1 2 3 15
	$(MKTSFS) -n 1 -i 1:1 -g 1 -m '$(TSFS_ROOT_MANIFEST)' \
	    -o '$(TSFS_ROOT_PREFIX)'

$(BOOT_MEDIA_STAMP): $(IMAGE_BOOT_WORDS) $(DISK_IMAGE) $(STAGE1_TAPE) $(ROOT_MEDIA_DEPS) | $(BUILD)
	@set -eu; \
	if [ '$(BOOT_PROFILE)' = lowmem ] && [ '$(STAGE1_DEVICE)' != dsk ]; then \
		echo 'BOOT_PROFILE=lowmem currently requires BOOT=dsk' >&2; exit 2; \
	fi; \
	mkdir -p '$(MEDIA_DIR)'; \
	touch '$(DTC0_IMAGE)' '$(DTC1_IMAGE)' '$(DTC2_IMAGE)' '$(MTC0_IMAGE)' '$(MTC1_IMAGE)' \
	    '$(DR0_IMAGE)' '$(DR1_IMAGE)' '$(DR2_IMAGE)' '$(DR3_IMAGE)'; \
	if [ '$(ROOT)' = drum ] || [ '$(STAGE1_DEVICE)' = drm ]; then \
		cp '$(DISK_IMAGE)' '$(DR0_IMAGE)'; \
	fi; \
	case '$(STAGE1_DEVICE)' in \
	  dsk|drm) : ;; \
	  dtc) '$(MKDT)' -t dtc -p '$(IMAGE_COMPRESSED_WORDS)' -o '$(DTC0_IMAGE)' ;; \
	  mtc) '$(MKTAP)' -t mtc -p '$(IMAGE_COMPRESSED_WORDS)' -o '$(MTC0_IMAGE)' ;; \
	  ptr) \
	    cwords=$$(tail -n +4 '$(IMAGE_COMPRESSED_WORDS)' | wc -l | tr -d ' '); \
	    first=$$(( (cwords + 1) / 2 )); second=$$(( cwords - first )); \
	    test $$first -gt 0; test $$second -gt 0; \
	    { printf '%012o\n' $$((first + 3)); head -n 3 '$(IMAGE_COMPRESSED_WORDS)'; \
	      tail -n +4 '$(IMAGE_COMPRESSED_WORDS)' | head -n $$first; } > '$(PTR_BOOT1_WORDS)'; \
	    { printf '%012o\n' $$second; tail -n +4 '$(IMAGE_COMPRESSED_WORDS)' | \
	      tail -n +$$((first + 1)); } > '$(PTR_BOOT2_WORDS)'; \
	    '$(WORDS2PT)' -t ptr -p '$(PTR_BOOT1_WORDS)' -o '$(PTR_BOOT1)'; \
	    '$(WORDS2PT)' -t ptr -p '$(PTR_BOOT2_WORDS)' -o '$(PTR_BOOT2)' ;; \
	esac; \
	if [ '$(ROOT)' = tape ]; then \
	  if [ '$(STAGE1_DEVICE)' = dtc ]; then \
	    cp '$(TSFS_ROOT_IMAGE)' '$(DTC1_IMAGE)'; \
	  else \
	    cp '$(TSFS_ROOT_IMAGE)' '$(DTC0_IMAGE)'; \
	  fi; \
	fi; \
	{ \
	  if [ '$(STAGE1_DEVICE)' = ptr ]; then \
	    echo 'break 060'; \
	    echo 'go 020'; \
	    echo 'nobreak 060'; \
	    echo 'detach ptr'; \
	    echo 'attach -q ptr $(PTR_BOOT1)'; \
	    echo 'deposit ptr time 1'; \
	    echo 'expect "TAPE2" detach ptr; attach -q ptr $(PTR_BOOT2); deposit ptr time 1; continue'; \
	    echo 'go 060'; \
	  else \
	    echo 'go 020'; \
	  fi; \
	} > '$(BOOT_COMMANDS)'; \
	touch '$@'

$(SIMH_INI): simh/simh.ini.in $(STAGE1_TAPE) $(DISK_IMAGE) $(BOOT_MEDIA_STAMP) force-simh-config
	@case '$(SIMH_PCLK_MODE)' in \
	  REALTIME|HISTORICAL) ;; \
	  *) echo "SIMH_PCLK_MODE must be REALTIME or HISTORICAL" >&2; exit 2 ;; \
	esac
	@case '$(ROOT)' in \
	  auto) root_class=0 ;; \
	  disk) root_class=1 ;; \
	  tape) root_class=2 ;; \
	  drum) root_class=3 ;; \
	  ram)  root_class=4 ;; \
	  *) echo "ROOT must be auto, disk, tape, drum, or ram" >&2; exit 2 ;; \
	esac; \
	case '$(ROOTSET)' in 1|2|3|4) ;; \
	  *) echo "ROOTSET must be 1, 2, 3, or 4" >&2; exit 2 ;; \
	esac; \
	root_switch=$$((root_class + (($(ROOTSET) - 1) << 3))); \
	printf '%o\n' $$root_switch > $(BUILD)/root-switch.tmp
	root_switch=`cat $(BUILD)/root-switch.tmp`; rm -f $(BUILD)/root-switch.tmp; \
	sed -e 's|@SIMH_LOG@|$(SIMH_LOG)|g' \
	    -e 's|@CPU_KWORDS@|$(SIMH_CPU_KWORDS)|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@DISK_IMAGE@|$(DISK_IMAGE)|g' \
	    -e 's|@DTC0_IMAGE@|$(DTC0_IMAGE)|g' \
	    -e 's|@DTC1_IMAGE@|$(DTC1_IMAGE)|g' \
	    -e 's|@DTC2_IMAGE@|$(DTC2_IMAGE)|g' \
	    -e 's|@MTC0_IMAGE@|$(MTC0_IMAGE)|g' \
	    -e 's|@MTC1_IMAGE@|$(MTC1_IMAGE)|g' \
	    -e 's|@DR0_IMAGE@|$(DR0_IMAGE)|g' \
	    -e 's|@DR1_IMAGE@|$(DR1_IMAGE)|g' \
	    -e 's|@DR2_IMAGE@|$(DR2_IMAGE)|g' \
	    -e 's|@DR3_IMAGE@|$(DR3_IMAGE)|g' \
	    -e 's|@PTP_OUTPUT@|$(PTP_OUTPUT)|g' \
	    -e 's|@CP_OUTPUT@|$(CP_OUTPUT)|g' \
	    -e 's|@LPT_OUTPUT@|$(LPT_OUTPUT)|g' \
	    -e 's|@DCS0_PORT@|$(SIMH_DCS0_PORT)|g' \
	    -e 's|@GE0_PORT@|$(SIMH_GE0_PORT)|g' \
	    -e 's|@PCLK_MODE@|$(SIMH_PCLK_MODE)|g' \
	    -e "s|@ROOT_SWITCH@|$$root_switch|g" $< > $@.tmp; \
	while IFS= read -r line || test -n "$$line"; do \
		if test "$$line" = "@BOOT_COMMANDS@"; then \
			cat '$(BOOT_COMMANDS)'; \
		else \
			printf '%s\n' "$$line"; \
		fi; \
	done < $@.tmp > $@; \
	rm -f $@.tmp

image: $(SIMH_INI)

permanent-size: $(KCORE_MAP) $(MRES_PACKAGE_OBJS)
	@./image/report-permanent.sh --kcore-map $(KCORE_MAP) --build $(BUILD) \
	    --objdump $(PDP10_OBJDUMP) --omit-blockset

boot:
	rm -rf $(DISK_DIR) $(SIMH_INI)
	$(MAKE) image
	$(SIMH_PDP6) $(SIMH_INI)

clean:
	rm -rf $(BUILD)
