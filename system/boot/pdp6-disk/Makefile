PROJECT_ROOT := $(abspath ../../..)
KERNEL_DIR := $(PROJECT_ROOT)/system/kernel
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
MKINITFS0 ?= $(PDP10_PREFIX)/bin/mkinitfs0
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
PDP10_OBJDUMP ?= $(PDP10_PREFIX)/bin/pdp10-objdump

D6FS_LOGSTORE_BLOCKS ?= 0400
D6FS_SWAP_TAIL_BLOCKS ?= 0200
DTFS_TENEX ?= 1
DTFS_ITS ?= 1
PROC_BOOT_USERS ?= 1
PROC_STACK_WATERMARK ?= 0

KINIT_BUILD := $(BUILD)/kinit-asm
KCORE_BUILD := $(BUILD)/kcore-asm
KINIT_STAMP := $(BUILD)/.kinit-built
KCORE_STAMP := $(BUILD)/.kcore-built
KERNEL_INPUTS := $(wildcard $(KERNEL_DIR)/*.c $(KERNEL_DIR)/*.h $(KERNEL_DIR)/*.s)

KINIT_NAMES = kinit kinit_diag module_minit kfs_boot kinit_boot d6fs_boot d6fs_super_boot diskset_boot logstore proc_boot proc_swap_boot kinit_io module_io bootbanner_generated
KINIT_LATE_NAMES = kinit_late_begin exec proc_late proc_swap_late mm_boot kinit_late kinit_late_end
KINIT_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_NAMES)))
KINIT_LATE_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_LATE_NAMES)))
KINIT_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_NAMES)))
KINIT_LATE_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_LATE_NAMES)))

KCORE_C_NAMES = vfs devicefs procfs mm module_runtime proc proc_swap file
KCORE_FIXED_NAMES = $(KCORE_C_NAMES) ret_pdp10 \
	dsk270_pdp10 kfmt_pdp10 vfs_pdp10 fs_mres_pdp10 file_pdp10 proc_pdp10 \
	device_state_pdp10 devicefs_pdp10 syscall_info_pdp10 \
	syscall_dispatch mach_user mach_enter_user kcore_io storage_router kcore_end
KCORE_FIXED_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(KCORE_FIXED_NAMES)))
KCORE_FIXED_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KCORE_FIXED_NAMES)))
MRES_NAMES = cty_io clk_io ptr_io ptp_io cr_io cp_io dcs_io ge_io dpy_io \
	tty_io wcnsls_io ocnsls_io dsk_io tape_io slv_io memfs_pdp10 \
	dtfs dtfs_pdp10 diskset diskset_dispatch d6fs d6fs_provider d6fs_validate_pdp10 d6fs_pdp10
MRES_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(MRES_NAMES)))

KCORE_DXR = $(BUILD)/kcore-v1.dxr
KCORE_MAP = $(BUILD)/kcore-v1.map
KCORE_RIM = $(BUILD)/kcore-v1.rim
KCORE_STREAM = $(BUILD)/kcore-v1.stream
KCORE_EMBED_ASM = $(BUILD)/kcore-embed-v1.s
KCORE_EMBED_OBJ = $(BUILD)/kcore-embed-v1.dobj

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
	$(WCNSLS_MRES_OBJ) $(OCNSLS_MRES_OBJ) $(DSK_MRES_OBJ) $(TAPE_MRES_OBJ) $(SLV_MRES_OBJ) \
	$(MEMFS_MRES_OBJ) $(DTFS_MRES_OBJ) $(DISKSET_MRES_OBJ) $(D6FS_MRES_OBJ)

MINIT_TABLE_OBJ = $(BUILD)/module-table-runtime.dobj
END_OBJ = $(BUILD)/image-end-v1.dobj
DISKBOOT_INITFS_ASM = $(BUILD)/initfs-diskboot-v1.s
DISKBOOT_INITFS_OBJ = $(BUILD)/initfs-diskboot-v1.dobj
IMAGE_DXR = $(BUILD)/kinit-v1.dxr
IMAGE_MAP = $(BUILD)/kinit-v1.map
IMAGE_RIM = $(BUILD)/kinit-v1.rim
IMAGE_RAW = $(BUILD)/kinit-v1.raw.words
IMAGE_WORDS = $(BUILD)/kinit-v1.words

USR_BUILD_ROOT = $(BUILD)/usr-build
DSH_DXR = $(USR_BUILD_ROOT)/usr/dsh.dxr
SYSTEM_INIT_DXR ?= $(DSH_DXR)
USR_INPUTS := $(wildcard $(PROJECT_ROOT)/usr/libc/* $(PROJECT_ROOT)/usr/exec/* \
	$(PROJECT_ROOT)/usr/dsh/*) $(PROJECT_ROOT)/usr/Makefile \
	$(KERNEL_DIR)/syscall.h

STAGE1_SRC = $(PROJECT_ROOT)/system/stand/pdp6/dsk/stage1.s
STAGE1_DXR = $(BUILD)/stage1-dsk-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dsk-v1.pt
DISK_DIR = $(BUILD)/disk
DISK_IMAGE = $(DISK_DIR)/dsk0.dsk
SIMH_INI = $(BUILD)/boot-v1.ini
SIMH_LOG = $(BUILD)/boot-v1.simh.log
MEDIA_DIR = $(BUILD)/media
DTC0_IMAGE = $(MEDIA_DIR)/dtc0.tap
DTC1_IMAGE = $(MEDIA_DIR)/dtc1.tap
DTC2_IMAGE = $(MEDIA_DIR)/dtc2.tap
MTC0_IMAGE = $(MEDIA_DIR)/mtc0.tap
MTC1_IMAGE = $(MEDIA_DIR)/mtc1.tap
PTP_OUTPUT = $(MEDIA_DIR)/ptp.out
CP_OUTPUT = $(MEDIA_DIR)/cp.out

.DEFAULT_GOAL := image
.PHONY: image boot permanent-size clean

$(BUILD):
	mkdir -p $@

$(KINIT_STAMP): $(KERNEL_INPUTS) | $(BUILD)
	$(MAKE) -C $(KERNEL_DIR) kinit BUILD='$(KINIT_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' PROC_STACK_WATERMARK='$(PROC_STACK_WATERMARK)'
	touch $@

$(KCORE_STAMP): $(KERNEL_INPUTS) | $(BUILD)
	$(MAKE) -C $(KERNEL_DIR) kcore BUILD='$(KCORE_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' DTFS_TENEX='$(DTFS_TENEX)' DTFS_ITS='$(DTFS_ITS)' PROC_BOOT_USERS='$(PROC_BOOT_USERS)' PROC_STACK_WATERMARK='$(PROC_STACK_WATERMARK)'
	touch $@

$(KINIT_ASM) $(KINIT_LATE_ASM): $(KINIT_STAMP)
	@test -f $@

$(KCORE_FIXED_ASM) $(MRES_ASM): $(KCORE_STAMP)
	@test -f $@

$(BUILD)/%.dobj: $(KINIT_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(BUILD)/%.dobj: $(KCORE_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(MINIT_TABLE_OBJ): $(KERNEL_DIR)/module_table.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(END_OBJ): image-end-v1.s | $(BUILD)
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

$(KCORE_EMBED_ASM): kcore-embed-v1.s.in $(KCORE_MAP) | $(BUILD)
	end=`awk '$$1 == "__kcore_low_init_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$end"; words=$$((0$$end - 060)); words=`printf '%o' "$$words"`; \
	sed "s/@KCORE_INIT_WORDS@/$$words/" $< > $@

$(KCORE_EMBED_OBJ): $(KCORE_EMBED_ASM)
	$(DAS) -c -o $@ $<

$(DSH_DXR): $(USR_INPUTS)
	$(MAKE) -C $(PROJECT_ROOT)/usr build BUILD_ROOT='$(USR_BUILD_ROOT)' \
	    PDP10_PREFIX='$(PDP10_PREFIX)'

# Diskboot reads /SYSTEM/INIT exclusively from D6FS.  KINIT carries only the
# empty bootstrap namespace header.
$(DISKBOOT_INITFS_ASM): | $(BUILD)
	$(MKINITFS0) --label __initfs_begin -o $@

$(DISKBOOT_INITFS_OBJ): $(DISKBOOT_INITFS_ASM)
	$(DAS) -c -o $@ $<

$(IMAGE_DXR): $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
    $(MRES_PACKAGE_OBJS) $(DISKBOOT_INITFS_OBJ) $(KINIT_LATE_OBJS) $(END_OBJ) $(KCORE_MAP)
	$(DLINK) -b 030000 -A $(KCORE_MAP) -o $@ -M $(IMAGE_MAP) \
	    $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
	    $(MRES_PACKAGE_OBJS) $(DISKBOOT_INITFS_OBJ) $(KINIT_LATE_OBJS) $(END_OBJ)

$(IMAGE_MAP): $(IMAGE_DXR)
	@test -f $@

$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 030000 $< $@

$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	end=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$end"; words=$$((0$$end - 030000)); words=`printf '%o' "$$words"`; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 030000 -w "$$words"

$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(KCORE_STREAM) $(KCORE_MAP) mkimage-v2.sh
	./mkimage-v2.sh --map $(IMAGE_MAP) --input $(IMAGE_RAW) \
	    --kcore $(KCORE_STREAM) --kcore-map $(KCORE_MAP) --output $@

$(STAGE1_DXR): $(STAGE1_SRC) | $(BUILD)
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

$(SIMH_INI): simh-v1.ini.in $(STAGE1_TAPE) $(DISK_IMAGE)
	mkdir -p $(MEDIA_DIR)
	touch $(DTC0_IMAGE) $(DTC1_IMAGE) $(DTC2_IMAGE) $(MTC0_IMAGE) $(MTC1_IMAGE)
	sed -e 's|@SIMH_LOG@|$(abspath $(SIMH_LOG))|g' \
	    -e 's|@STAGE1_TAPE@|$(abspath $(STAGE1_TAPE))|g' \
	    -e 's|@DISK_IMAGE@|$(abspath $(DISK_IMAGE))|g' \
	    -e 's|@DTC0_IMAGE@|$(abspath $(DTC0_IMAGE))|g' \
	    -e 's|@DTC1_IMAGE@|$(abspath $(DTC1_IMAGE))|g' \
	    -e 's|@DTC2_IMAGE@|$(abspath $(DTC2_IMAGE))|g' \
	    -e 's|@MTC0_IMAGE@|$(abspath $(MTC0_IMAGE))|g' \
	    -e 's|@MTC1_IMAGE@|$(abspath $(MTC1_IMAGE))|g' \
	    -e 's|@PTP_OUTPUT@|$(abspath $(PTP_OUTPUT))|g' \
	    -e 's|@CP_OUTPUT@|$(abspath $(CP_OUTPUT))|g' $< > $@

image: $(SIMH_INI)

permanent-size: $(KCORE_MAP) $(MRES_PACKAGE_OBJS)
	@./report-permanent-v1.sh --kcore-map $(KCORE_MAP) --build $(BUILD) --objdump $(PDP10_OBJDUMP)

boot:
	rm -rf $(DISK_DIR) $(SIMH_INI)
	$(MAKE) image
	$(SIMH_PDP6) $(SIMH_INI)

clean:
	rm -rf $(BUILD)
