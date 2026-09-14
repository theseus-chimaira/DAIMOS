PROJECT_ROOT != cd ../../.. && pwd
KERNEL_DIR = $(PROJECT_ROOT)/system/kernel
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/system/boot/pdp6-disk
PDP10_PREFIX ?= $(HOME)/cross

DAS ?= $(PDP10_PREFIX)/bin/das
DLINK ?= $(PDP10_PREFIX)/bin/dlink
DXRCONVERT ?= $(PDP10_PREFIX)/bin/dxrconvert
MKSTREAM ?= $(PDP10_PREFIX)/bin/mkstream
MKDSK ?= $(PDP10_PREFIX)/bin/mkdsk
MKD6FS ?= $(PDP10_PREFIX)/bin/mkd6fs
LOGSTORE ?= $(PDP10_PREFIX)/bin/logstore
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
PDP10_OBJDUMP ?= $(PDP10_PREFIX)/bin/pdp10-objdump

D6FS_LOGSTORE_BLOCKS ?= 0400
D6FS_SWAP_TAIL_BLOCKS ?= 0200
DTFS_TENEX ?= 0
DTFS_ITS ?= 0
PROC_BOOT_USERS ?= 1
PROC_STACK_WATERMARK ?= 0
VM_BACKEND ?= pdp6

KINIT_BUILD = $(BUILD)/kinit-asm
KCORE_BUILD = $(BUILD)/kcore-asm
KINIT_STAMP = $(BUILD)/.kinit-built
KCORE_STAMP = $(BUILD)/.kcore-built
KERNEL_INPUTS = $(KERNEL_DIR)/Makefile \
	$(KERNEL_DIR)/boot/kboot.h \
	$(KERNEL_DIR)/boot/kfs_boot.c \
	$(KERNEL_DIR)/boot/kinit.c \
	$(KERNEL_DIR)/boot/kinit.h \
	$(KERNEL_DIR)/boot/kinit_boot.c \
	$(KERNEL_DIR)/boot/kinit_diag.c \
	$(KERNEL_DIR)/boot/kinit_io.s \
	$(KERNEL_DIR)/boot/kinit_late.c \
	$(KERNEL_DIR)/boot/kinit_late_begin.s \
	$(KERNEL_DIR)/boot/kinit_late_end.s \
	$(KERNEL_DIR)/core/kcore.h \
	$(KERNEL_DIR)/core/kcore_end.s \
	$(KERNEL_DIR)/core/kcore_io.s \
	$(KERNEL_DIR)/core/kcore_pi.h \
	$(KERNEL_DIR)/core/kfmt.h \
	$(KERNEL_DIR)/core/kfmt_pdp10.s \
	$(KERNEL_DIR)/core/ret_pdp10.s \
	$(KERNEL_DIR)/drivers/card.h \
	$(KERNEL_DIR)/drivers/clk.h \
	$(KERNEL_DIR)/drivers/clk_io.s \
	$(KERNEL_DIR)/drivers/cp_io.s \
	$(KERNEL_DIR)/drivers/cr_io.s \
	$(KERNEL_DIR)/drivers/cty.h \
	$(KERNEL_DIR)/drivers/cty_io.s \
	$(KERNEL_DIR)/drivers/dcs.h \
	$(KERNEL_DIR)/drivers/dcs_io.s \
	$(KERNEL_DIR)/drivers/device_state_pdp10.s \
	$(KERNEL_DIR)/drivers/dpy.h \
	$(KERNEL_DIR)/drivers/dpy_io.s \
	$(KERNEL_DIR)/drivers/dsk270.h \
	$(KERNEL_DIR)/drivers/dsk270_pdp10.s \
	$(KERNEL_DIR)/drivers/dsk_io.s \
	$(KERNEL_DIR)/drivers/ge.h \
	$(KERNEL_DIR)/drivers/ge_io.s \
	$(KERNEL_DIR)/drivers/ocnsls.h \
	$(KERNEL_DIR)/drivers/ocnsls_io.s \
	$(KERNEL_DIR)/drivers/pt.h \
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
	$(KERNEL_DIR)/fs/d6fs_boot.c \
	$(KERNEL_DIR)/fs/d6fs_boot.h \
	$(KERNEL_DIR)/fs/d6fs_mres.c \
	$(KERNEL_DIR)/fs/d6fs_pdp10.s \
	$(KERNEL_DIR)/fs/d6fs_provider.c \
	$(KERNEL_DIR)/fs/d6fs_provider.h \
	$(KERNEL_DIR)/fs/d6fs_provider_mutation_pdp10.s \
	$(KERNEL_DIR)/fs/d6fs_super_boot.c \
	$(KERNEL_DIR)/fs/d6fs_validate_pdp10.s \
	$(KERNEL_DIR)/fs/devicefs.c \
	$(KERNEL_DIR)/fs/devicefs.h \
	$(KERNEL_DIR)/fs/domainfs.c \
	$(KERNEL_DIR)/fs/devicefs_pdp10.s \
	$(KERNEL_DIR)/fs/dtfs.c \
	$(KERNEL_DIR)/fs/dtfs.h \
	$(KERNEL_DIR)/fs/dtfs_chain_walk_pdp10.s \
	$(KERNEL_DIR)/fs/dtfs_pdp10.s \
	$(KERNEL_DIR)/fs/dtfs_resize_pdp10.s \
	$(KERNEL_DIR)/fs/file.c \
	$(KERNEL_DIR)/fs/file.h \
	$(KERNEL_DIR)/fs/file_pdp10.s \
	$(KERNEL_DIR)/fs/fs_mres.h \
	$(KERNEL_DIR)/fs/fs_mres_pdp10.s \
	$(KERNEL_DIR)/fs/memfs.c \
	$(KERNEL_DIR)/fs/memfs.h \
	$(KERNEL_DIR)/fs/memfs_pdp10.s \
	$(KERNEL_DIR)/fs/procfs.c \
	$(KERNEL_DIR)/fs/procfs.h \
	$(KERNEL_DIR)/fs/procfs_pdp10.s \
	$(KERNEL_DIR)/fs/domainfs_pdp10.s \
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
	$(KERNEL_DIR)/storage/diskset.h \
	$(KERNEL_DIR)/storage/diskset_boot.c \
	$(KERNEL_DIR)/storage/diskset_boot.h \
	$(KERNEL_DIR)/storage/diskset_dispatch.s \
	$(KERNEL_DIR)/storage/diskset_layout.h \
	$(KERNEL_DIR)/storage/diskset_mres.h \
	$(KERNEL_DIR)/storage/logstore.c \
	$(KERNEL_DIR)/storage/logstore.h \
	$(KERNEL_DIR)/storage/logstore_drain.c \
	$(KERNEL_DIR)/storage/logstore_mtc_sink.c \
	$(KERNEL_DIR)/storage/storage.h \
	$(KERNEL_DIR)/storage/storage_router.s

KINIT_NAMES = kinit kinit_diag module_minit kfs_boot kinit_boot d6fs_boot d6fs_super_boot diskset_boot logstore proc_boot proc_swap_boot kinit_io module_io bootbanner_generated
KINIT_LATE_NAMES = kinit_late_begin proc_late mm_boot kinit_late kinit_late_end
KINIT_ASM = $(KINIT_BUILD)/kinit.s $(KINIT_BUILD)/kinit_diag.s $(KINIT_BUILD)/module_minit.s $(KINIT_BUILD)/kfs_boot.s $(KINIT_BUILD)/kinit_boot.s $(KINIT_BUILD)/d6fs_boot.s $(KINIT_BUILD)/d6fs_super_boot.s $(KINIT_BUILD)/diskset_boot.s $(KINIT_BUILD)/logstore.s $(KINIT_BUILD)/proc_boot.s $(KINIT_BUILD)/proc_swap_boot.s $(KINIT_BUILD)/kinit_io.s $(KINIT_BUILD)/module_io.s $(KINIT_BUILD)/bootbanner_generated.s
KINIT_LATE_ASM = $(KINIT_BUILD)/kinit_late_begin.s $(KINIT_BUILD)/proc_late.s $(KINIT_BUILD)/mm_boot.s $(KINIT_BUILD)/kinit_late.s $(KINIT_BUILD)/kinit_late_end.s
KINIT_OBJS = $(BUILD)/kinit.dobj $(BUILD)/kinit_diag.dobj $(BUILD)/module_minit.dobj $(BUILD)/kfs_boot.dobj $(BUILD)/kinit_boot.dobj $(BUILD)/d6fs_boot.dobj $(BUILD)/d6fs_super_boot.dobj $(BUILD)/diskset_boot.dobj $(BUILD)/logstore.dobj $(BUILD)/proc_boot.dobj $(BUILD)/proc_swap_boot.dobj $(BUILD)/kinit_io.dobj $(BUILD)/module_io.dobj $(BUILD)/bootbanner_generated.dobj
KINIT_LATE_OBJS = $(BUILD)/kinit_late_begin.dobj $(BUILD)/proc_late.dobj $(BUILD)/mm_boot.dobj $(BUILD)/kinit_late.dobj $(BUILD)/kinit_late_end.dobj

KCORE_C_NAMES = exec vfs devicefs mm module_runtime proc vm_backend proc_swap file
KCORE_FIXED_NAMES = $(KCORE_C_NAMES) ret_pdp10 \
	dsk270_pdp10 kfmt_pdp10 vfs_pdp10 fs_mres_pdp10 file_pdp10 procfs_pdp10 domainfs_pdp10 proc_pdp10 proc_run_pdp10 \
	device_state_pdp10 devicefs_pdp10 syscall_info_pdp10 \
	syscall_dispatch mach_user vm_backend_asm kcore_io storage_router kcore_end
KCORE_FIXED_ASM = $(KCORE_BUILD)/exec.s $(KCORE_BUILD)/devicefs.s $(KCORE_BUILD)/mm.s $(KCORE_BUILD)/module_runtime.s $(KCORE_BUILD)/proc.s $(KCORE_BUILD)/vm_backend.s $(KCORE_BUILD)/proc_swap.s $(KCORE_BUILD)/file.s $(KCORE_BUILD)/ret_pdp10.s $(KCORE_BUILD)/dsk270_pdp10.s $(KCORE_BUILD)/kfmt_pdp10.s $(KCORE_BUILD)/vfs_pdp10.s $(KCORE_BUILD)/fs_mres_pdp10.s $(KCORE_BUILD)/file_pdp10.s $(KCORE_BUILD)/pipe_pdp10.s $(KCORE_BUILD)/procfs_pdp10.s $(KCORE_BUILD)/domainfs_pdp10.s $(KCORE_BUILD)/proc_pdp10.s $(KCORE_BUILD)/proc_run_pdp10.s $(KCORE_BUILD)/device_state_pdp10.s $(KCORE_BUILD)/devicefs_pdp10.s $(KCORE_BUILD)/syscall_info_pdp10.s $(KCORE_BUILD)/syscall_dispatch.s $(KCORE_BUILD)/mach_user.s $(KCORE_BUILD)/vm_backend_asm.s $(KCORE_BUILD)/kcore_io.s $(KCORE_BUILD)/storage_router.s $(KCORE_BUILD)/kcore_end.s
KCORE_FIXED_OBJS = $(BUILD)/exec.dobj $(BUILD)/devicefs.dobj $(BUILD)/mm.dobj $(BUILD)/module_runtime.dobj $(BUILD)/proc.dobj $(BUILD)/vm_backend.dobj $(BUILD)/proc_swap.dobj $(BUILD)/file.dobj $(BUILD)/ret_pdp10.dobj $(BUILD)/dsk270_pdp10.dobj $(BUILD)/kfmt_pdp10.dobj $(BUILD)/vfs_pdp10.dobj $(BUILD)/fs_mres_pdp10.dobj $(BUILD)/file_pdp10.dobj $(BUILD)/pipe_pdp10.dobj $(BUILD)/procfs_pdp10.dobj $(BUILD)/domainfs_pdp10.dobj $(BUILD)/proc_pdp10.dobj $(BUILD)/proc_run_pdp10.dobj $(BUILD)/device_state_pdp10.dobj $(BUILD)/devicefs_pdp10.dobj $(BUILD)/syscall_info_pdp10.dobj $(BUILD)/syscall_dispatch.dobj $(BUILD)/mach_user.dobj $(BUILD)/vm_backend_asm.dobj $(BUILD)/kcore_io.dobj $(BUILD)/storage_router.dobj $(BUILD)/kcore_end.dobj
MRES_NAMES = cty_io clk_io ptr_io ptp_io cr_io cp_io dcs_io ge_io dpy_io \
	tty_io wcnsls_io ocnsls_io dsk_io tape_io slv_io memfs_pdp10 \
	dtfs dtfs_pdp10 diskset_dispatch d6fs d6fs_provider d6fs_validate_pdp10 d6fs_pdp10
MRES_ASM = $(KCORE_BUILD)/cty_io.s $(KCORE_BUILD)/clk_io.s $(KCORE_BUILD)/ptr_io.s $(KCORE_BUILD)/ptp_io.s $(KCORE_BUILD)/cr_io.s $(KCORE_BUILD)/cp_io.s $(KCORE_BUILD)/dcs_io.s $(KCORE_BUILD)/ge_io.s $(KCORE_BUILD)/dpy_io.s $(KCORE_BUILD)/tty_io.s $(KCORE_BUILD)/wcnsls_io.s $(KCORE_BUILD)/ocnsls_io.s $(KCORE_BUILD)/dsk_io.s $(KCORE_BUILD)/tape_io.s $(KCORE_BUILD)/slv_io.s $(KCORE_BUILD)/memfs_pdp10.s $(KCORE_BUILD)/dtfs.s $(KCORE_BUILD)/dtfs_pdp10.s $(KCORE_BUILD)/diskset_dispatch.s $(KCORE_BUILD)/d6fs.s $(KCORE_BUILD)/d6fs_provider.s $(KCORE_BUILD)/d6fs_validate_pdp10.s $(KCORE_BUILD)/d6fs_pdp10.s

KCORE_DXR = $(BUILD)/kcore.dxr
KCORE_MAP = $(BUILD)/kcore.map
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
TAPE_MRES_OBJ = $(BUILD)/tape-mres.dobj
SLV_MRES_OBJ = $(BUILD)/slv-mres.dobj
MEMFS_MRES_OBJ = $(BUILD)/memfs-mres.dobj
DTFS_MRES_OBJ = $(BUILD)/dtfs-mres.dobj
DISKSET_MRES_OBJ = $(BUILD)/diskset-mres.dobj
D6FS_MRES_OBJ = $(BUILD)/d6fs-mres.dobj
MRES_PACKAGE_OBJS = $(CTY_MRES_OBJ) $(CLK_MRES_OBJ) $(PTR_MRES_OBJ) \
	$(PTP_MRES_OBJ) $(CR_MRES_OBJ) $(CP_MRES_OBJ) \
	$(DCS_MRES_OBJ) $(GE_MRES_OBJ) $(DPY_MRES_OBJ) $(TTY_MRES_OBJ) \
	$(WCNSLS_MRES_OBJ) $(OCNSLS_MRES_OBJ) $(TAPE_MRES_OBJ) $(DSK_MRES_OBJ) \
	$(DISKSET_MRES_OBJ) $(MEMFS_MRES_OBJ) $(DTFS_MRES_OBJ) $(D6FS_MRES_OBJ) \
	$(SLV_MRES_OBJ)

MINIT_TABLE_OBJ = $(BUILD)/module-table-runtime.dobj
END_OBJ = $(BUILD)/image-end.dobj
IMAGE_DXR = $(BUILD)/kinit.dxr
IMAGE_MAP = $(BUILD)/kinit.map
IMAGE_RIM = $(BUILD)/kinit.rim
IMAGE_RAW = $(BUILD)/kinit.raw.words
IMAGE_WORDS = $(BUILD)/kinit.words

USERLAND_BUILD_ROOT = $(BUILD)/userland-build
DSH_DXR = $(USERLAND_BUILD_ROOT)/userland/dsh.dxr
SYSTEM_INIT_DXR ?= $(DSH_DXR)
USERLAND_INPUTS = $(PROJECT_ROOT)/userland/libc/Makefile \
	$(PROJECT_ROOT)/userland/libc/crt0.s \
	$(PROJECT_ROOT)/userland/libc/dsys.h \
	$(PROJECT_ROOT)/userland/libc/memcpy.s \
	$(PROJECT_ROOT)/userland/libc/memmove.s \
	$(PROJECT_ROOT)/userland/libc/syscall.s \
	$(PROJECT_ROOT)/userland/libc/syscall_helpers.s \
	$(PROJECT_ROOT)/userland/libc/u.c \
	$(PROJECT_ROOT)/userland/libc/u.h \
	$(PROJECT_ROOT)/userland/exec/COMMANDS \
	$(PROJECT_ROOT)/userland/exec/cmd.h \
	$(PROJECT_ROOT)/userland/exec/commands.c \
	$(PROJECT_ROOT)/userland/exec/standalone.c \
	$(PROJECT_ROOT)/userland/dsh/dsh.c \
	$(PROJECT_ROOT)/userland/Makefile \
	$(PROJECT_ROOT)/system/kernel/proc/syscall.h

STAGE1_SRC = $(PROJECT_ROOT)/system/stand/pdp6/dsk/stage1.s
STAGE1_DXR = $(BUILD)/stage1-dsk.dxr
STAGE1_TAPE = $(BUILD)/stage1-dsk.pt
DISK_DIR = $(BUILD)/disk
DISK_IMAGE = $(DISK_DIR)/dsk0.dsk
SIMH_INI = $(BUILD)/boot.ini
SIMH_LOG = $(BUILD)/boot.simh.log
MEDIA_DIR = $(BUILD)/media
DTC0_IMAGE = $(MEDIA_DIR)/dtc0.tap
DTC1_IMAGE = $(MEDIA_DIR)/dtc1.tap
DTC2_IMAGE = $(MEDIA_DIR)/dtc2.tap
MTC0_IMAGE = $(MEDIA_DIR)/mtc0.tap
MTC1_IMAGE = $(MEDIA_DIR)/mtc1.tap
PTP_OUTPUT = $(MEDIA_DIR)/ptp.out
CP_OUTPUT = $(MEDIA_DIR)/cp.out

.PHONY: image boot permanent-size clean

$(BUILD):
	mkdir -p $@

$(KINIT_STAMP): $(KERNEL_INPUTS) $(BUILD)
	$(MAKE) -C $(KERNEL_DIR) kinit BUILD='$(KINIT_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' PROC_STACK_WATERMARK='$(PROC_STACK_WATERMARK)' VM_BACKEND='$(VM_BACKEND)'
	touch $@

$(KCORE_STAMP): $(KERNEL_INPUTS) $(BUILD)
	$(MAKE) -C $(KERNEL_DIR) kcore BUILD='$(KCORE_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' DTFS_TENEX='$(DTFS_TENEX)' DTFS_ITS='$(DTFS_ITS)' PROC_BOOT_USERS='$(PROC_BOOT_USERS)' PROC_STACK_WATERMARK='$(PROC_STACK_WATERMARK)' VM_BACKEND='$(VM_BACKEND)'
	touch $@

$(KINIT_ASM) $(KINIT_LATE_ASM): $(KINIT_STAMP)
	@test -f $@

$(KCORE_FIXED_ASM) $(MRES_ASM): $(KCORE_STAMP)
	@test -f $@

KERNEL_ASM_OBJS = $(BUILD)/kinit.dobj $(BUILD)/kinit_diag.dobj $(BUILD)/module_minit.dobj $(BUILD)/kfs_boot.dobj $(BUILD)/kinit_boot.dobj $(BUILD)/d6fs_boot.dobj $(BUILD)/d6fs_super_boot.dobj $(BUILD)/diskset_boot.dobj $(BUILD)/logstore.dobj $(BUILD)/proc_boot.dobj $(BUILD)/proc_swap_boot.dobj $(BUILD)/kinit_io.dobj $(BUILD)/module_io.dobj $(BUILD)/bootbanner_generated.dobj $(BUILD)/kinit_late_begin.dobj $(BUILD)/proc_late.dobj $(BUILD)/mm_boot.dobj $(BUILD)/kinit_late.dobj $(BUILD)/kinit_late_end.dobj $(BUILD)/exec.dobj $(BUILD)/devicefs.dobj $(BUILD)/mm.dobj $(BUILD)/module_runtime.dobj $(BUILD)/proc.dobj $(BUILD)/vm_backend.dobj $(BUILD)/proc_swap.dobj $(BUILD)/file.dobj $(BUILD)/ret_pdp10.dobj $(BUILD)/dsk270_pdp10.dobj $(BUILD)/kfmt_pdp10.dobj $(BUILD)/vfs_pdp10.dobj $(BUILD)/fs_mres_pdp10.dobj $(BUILD)/file_pdp10.dobj $(BUILD)/pipe_pdp10.dobj $(BUILD)/procfs_pdp10.dobj $(BUILD)/domainfs_pdp10.dobj $(BUILD)/proc_pdp10.dobj $(BUILD)/proc_run_pdp10.dobj $(BUILD)/device_state_pdp10.dobj $(BUILD)/devicefs_pdp10.dobj $(BUILD)/syscall_info_pdp10.dobj $(BUILD)/syscall_dispatch.dobj $(BUILD)/mach_user.dobj $(BUILD)/vm_backend_asm.dobj $(BUILD)/kcore_io.dobj $(BUILD)/storage_router.dobj $(BUILD)/kcore_end.dobj $(BUILD)/cty_io.dobj $(BUILD)/clk_io.dobj $(BUILD)/ptr_io.dobj $(BUILD)/ptp_io.dobj $(BUILD)/cr_io.dobj $(BUILD)/cp_io.dobj $(BUILD)/dcs_io.dobj $(BUILD)/ge_io.dobj $(BUILD)/dpy_io.dobj $(BUILD)/tty_io.dobj $(BUILD)/wcnsls_io.dobj $(BUILD)/ocnsls_io.dobj $(BUILD)/dsk_io.dobj $(BUILD)/tape_io.dobj $(BUILD)/slv_io.dobj $(BUILD)/memfs_pdp10.dobj $(BUILD)/dtfs.dobj $(BUILD)/dtfs_pdp10.dobj $(BUILD)/diskset_dispatch.dobj $(BUILD)/d6fs.dobj $(BUILD)/d6fs_provider.dobj $(BUILD)/d6fs_validate_pdp10.dobj $(BUILD)/d6fs_pdp10.dobj

$(KERNEL_ASM_OBJS): $(KINIT_ASM) $(KINIT_LATE_ASM) $(KCORE_FIXED_ASM) $(MRES_ASM)
	@mkdir -p $(BUILD)
	@base=`basename "$@" .dobj`; src=; if test -f "$(KINIT_BUILD)/$$base.s"; then src="$(KINIT_BUILD)/$$base.s"; elif test -f "$(KCORE_BUILD)/$$base.s"; then src="$(KCORE_BUILD)/$$base.s"; fi; test -n "$$src"; $(DAS) -c -o "$@" "$$src"

$(MINIT_TABLE_OBJ): $(KERNEL_DIR)/modules/module_table.s $(BUILD)
	$(DAS) -c -o $@ $<

$(END_OBJ): image-end.s $(BUILD)
	$(DAS) -c -o $@ $<

$(KCORE_DXR): $(KCORE_FIXED_OBJS)
	libgcc=`$(PDP10_GCC) -print-libgcc-file-name`; \
	$(DLINK) -b 060 -o $@ -M $(KCORE_MAP) $(KCORE_FIXED_OBJS) $$libgcc

$(KCORE_MAP): $(KCORE_DXR)
	@test -f $@

$(KCORE_RIM): $(KCORE_DXR)
	$(DXRCONVERT) --simh -b 060 $< $@

$(KCORE_STREAM): $(KCORE_RIM) $(KCORE_MAP)
	end=`awk '$$1 == "__kcore_low_init_end" { print $$2 }' $(KCORE_MAP)`; \
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
	    -R $@ -N tty_mres_package -X tty_putchar -X tty_cty_putchar_address \
	    -X tty_dcs_putchar_address -X tty_ge_putchar_address $(BUILD)/tty_io.dobj

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
	    -X dsk_write_sector -X dsk_dct_handler $(BUILD)/dsk_io.dobj

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

$(DTFS_MRES_OBJ): $(BUILD)/dtfs.dobj $(BUILD)/dtfs_pdp10.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/dtfs-mres.dxr -M $(BUILD)/dtfs-mres.map \
	    -R $@ -N dtfs_mres_package -X dtfs_mres_dispatch \
	    -X dtfs_dtc_read_jump -X dtfs_dtc_write_jump \
	    -X dtfs_format_unit -X dtfs_mount_unit \
	    $(BUILD)/dtfs.dobj $(BUILD)/dtfs_pdp10.dobj

$(DISKSET_MRES_OBJ): $(BUILD)/diskset_dispatch.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/diskset-mres.dxr -M $(BUILD)/diskset-mres.map \
	    -R $@ -N diskset_mres_package -X diskset_mres_dispatch \
	    -X diskset_boot -X diskset_total_blocks \
	    -X diskset_read_block -X diskset_write_block \
	    $(BUILD)/diskset_dispatch.dobj

$(D6FS_MRES_OBJ): $(BUILD)/d6fs.dobj $(BUILD)/d6fs_provider.dobj $(BUILD)/d6fs_validate_pdp10.dobj $(BUILD)/d6fs_pdp10.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/d6fs-mres.dxr -M $(BUILD)/d6fs-mres.map \
	    -R $@ -N d6fs_mres_package -X d6fs_mres_dispatch \
	    -X d6fs_diskset_read -X d6fs_diskset_write \
	    -X d6fs_provider_reader \
	    -X d6fs_diskset_read_jump -X d6fs_diskset_write_jump \
	    $(BUILD)/d6fs.dobj $(BUILD)/d6fs_provider.dobj $(BUILD)/d6fs_validate_pdp10.dobj $(BUILD)/d6fs_pdp10.dobj

$(KCORE_EMBED_ASM): kcore-embed.s.in $(KCORE_MAP) $(BUILD)
	end=`awk '$$1 == "__kcore_low_init_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$end"; words=$$((0$$end - 060)); words=`printf '%o' "$$words"`; \
	sed "s/@KCORE_INIT_WORDS@/$$words/" $< > $@

$(KCORE_EMBED_OBJ): $(KCORE_EMBED_ASM)
	$(DAS) -c -o $@ $<

$(DSH_DXR): $(USERLAND_INPUTS)
	$(MAKE) -C $(PROJECT_ROOT)/userland build BUILD_ROOT='$(USERLAND_BUILD_ROOT)' \
	    PDP10_PREFIX='$(PDP10_PREFIX)'

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
	end=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$end"; words=$$((0$$end - 030000)); words=`printf '%o' "$$words"`; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 030000 -w "$$words"

$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(KCORE_STREAM) $(KCORE_MAP) mkimage.sh
	./mkimage.sh --map $(IMAGE_MAP) --input $(IMAGE_RAW) \
	    --kcore $(KCORE_STREAM) --kcore-map $(KCORE_MAP) --output $@

$(STAGE1_DXR): $(STAGE1_SRC) $(BUILD)
	$(DAS) --base-kernel -o $@ $<

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b 060 $< $@
	printf '\252' >> $@

$(DISK_IMAGE): $(IMAGE_WORDS) $(SYSTEM_INIT_DXR)
	rm -rf $(DISK_DIR)
	$(MKDSK) -n 1 -m clean -p $(IMAGE_WORDS) -o $(DISK_DIR) \
	    --d6fs-layout --logstore-blocks $(D6FS_LOGSTORE_BLOCKS) \
	    --swap-tail-blocks $(D6FS_SWAP_TAIL_BLOCKS)
	$(MKD6FS) -n 1 -d $(DISK_DIR) \
	    -D /MOUNT/RAMFS0:0755 -D /TEMP:0555 \
	    -D /MOUNT/DT0:0755 -D /MOUNT/DT1:0755 -D /MOUNT/DT2:0755 \
	    -f "/SYSTEM/INIT:$(SYSTEM_INIT_DXR):555:dxr"
	$(LOGSTORE) -n 1 -d $(DISK_DIR) --init

$(SIMH_INI): simh.ini.in $(STAGE1_TAPE) $(DISK_IMAGE)
	mkdir -p $(MEDIA_DIR)
	touch $(DTC0_IMAGE) $(DTC1_IMAGE) $(DTC2_IMAGE) $(MTC0_IMAGE) $(MTC1_IMAGE)
	sed -e 's|@SIMH_LOG@|$(SIMH_LOG)|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@DISK_IMAGE@|$(DISK_IMAGE)|g' \
	    -e 's|@DTC0_IMAGE@|$(DTC0_IMAGE)|g' \
	    -e 's|@DTC1_IMAGE@|$(DTC1_IMAGE)|g' \
	    -e 's|@DTC2_IMAGE@|$(DTC2_IMAGE)|g' \
	    -e 's|@MTC0_IMAGE@|$(MTC0_IMAGE)|g' \
	    -e 's|@MTC1_IMAGE@|$(MTC1_IMAGE)|g' \
	    -e 's|@PTP_OUTPUT@|$(PTP_OUTPUT)|g' \
	    -e 's|@CP_OUTPUT@|$(CP_OUTPUT)|g' $< > $@

image: $(SIMH_INI)

permanent-size: $(KCORE_MAP) $(MRES_PACKAGE_OBJS)
	@./report-permanent.sh --kcore-map $(KCORE_MAP) --build $(BUILD) --objdump $(PDP10_OBJDUMP)

boot:
	rm -rf $(DISK_DIR) $(SIMH_INI)
	$(MAKE) image
	$(SIMH_PDP6) $(SIMH_INI)

clean:
	rm -rf $(BUILD)
