PROJECT_ROOT := $(abspath ../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/kernel
KINIT_DEBUG ?= 0
KINIT_VARIANT := $(if $(filter 1,$(KINIT_DEBUG)),debug,normal)
KINIT_BUILD := $(BUILD)/kinit-asm-$(KINIT_VARIANT)

KCORE_BUILD := $(BUILD)/kcore-asm
PDP10_PREFIX ?= $(HOME)/cross

DAS ?= $(PDP10_PREFIX)/bin/das
DLINK ?= $(PDP10_PREFIX)/bin/dlink
DXRCONVERT ?= $(PDP10_PREFIX)/bin/dxrconvert
MKSTREAM ?= $(PDP10_PREFIX)/bin/mkstream
MKDSK ?= $(PDP10_PREFIX)/bin/mkdsk
MKINITFS0 ?= $(PDP10_PREFIX)/bin/mkinitfs0
MKTAP ?= $(PDP10_PREFIX)/bin/mktap
MKDT ?= $(PDP10_PREFIX)/bin/mkdt
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
PYTHON ?= python3
HOST_CC ?= cc
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
STDBUF ?= stdbuf
WATCH_OUTPUT := $(PROJECT_ROOT)/tests/simh/watch-output.sh

KINIT_NAMES = kinit kinit_diag module_minit kfs_boot_v2 kinit_io module_io
KINIT_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_NAMES)))
KINIT_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_NAMES)))
KCORE_C_NAMES = kboot_v1 vfs_v1 devicefs_v1 procfs_v1 memfs_v1 file_v1 proc_v1 sched_v1 syscall_v1 exec_v1
KCORE_FIXED_NAMES = kcore_entry $(KCORE_C_NAMES) syscall_dispatch_v2 mach_user_v1 kcore_io kcore_end
KCORE_FIXED_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(KCORE_FIXED_NAMES)))
KCORE_FIXED_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KCORE_FIXED_NAMES)))
MRES_NAMES = cty_io clk_io ptr_io ptp_io cr_io cp_io io7_io dcs_io ge_io dpy_io tty_io wcnsls_io ocnsls_io storage_io slv_io
MRES_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(MRES_NAMES)))

KINIT_INPUTS = $(addprefix $(PROJECT_ROOT)/system/kernel/, \
	kinit.c kinit_diag.c module_minit.c kfs_boot_v2.c kinit.h module.h mres.h \
	cty.h clk.h pt.h card.h dcs.h ge.h dpy.h tty.h wcnsls.h ocnsls.h kinit_io.s module_io.s)
KCORE_INPUTS = $(addprefix $(PROJECT_ROOT)/system/kernel/, \
	kcore.h kcore_pi.h kboot_v1.h vfs_v1.h devicefs_v1.h procfs_v1.h memfs_v1.h initfs_v1.h ramfs_v1.h file_v1.h proc_v1.h sched_v1.h syscall_v1.h exec_v1.h mach_user_v1.h \
	cty.h clk.h pt.h card.h dcs.h ge.h dpy.h tty.h wcnsls.h ocnsls.h storage.h slv.h \
	kcore_entry.s kboot_v1.c vfs_v1.c devicefs_v1.c procfs_v1.c memfs_v1.c initfs_v1.c ramfs_v1.c file_v1.c proc_v1.c sched_v1.c syscall_v1.c exec_v1.c syscall_dispatch_v2.s mach_user_v1.s kcore_io.s kcore_end.s \
	cty_io.s clk_io.s ptr_io.s ptp_io.s cr_io.s cp_io.s dcs_io.s ge_io.s dpy_io.s tty_io.s wcnsls_io.s ocnsls_io.s storage_io.s slv_io.s)

KCORE_DXR = $(BUILD)/kcore-v1.dxr
KCORE_MAP = $(BUILD)/kcore-v1.map
KCORE_RIM = $(BUILD)/kcore-v1.rim
KCORE_STREAM = $(BUILD)/kcore-v1.stream
KCORE_EMBED_ASM = $(BUILD)/kcore-embed-v1.s
KCORE_EMBED_OBJ = $(BUILD)/kcore-embed-v1.dobj

CTY_MRES_OBJ = $(BUILD)/cty-mres.dobj
CLK_MRES_OBJ = $(BUILD)/clk-mres.dobj
IO7_MRES_OBJ = $(BUILD)/io7-mres.dobj
DCS_MRES_OBJ = $(BUILD)/dcs-mres.dobj
GE_MRES_OBJ = $(BUILD)/ge-mres.dobj
DPY_MRES_OBJ = $(BUILD)/dpy-mres.dobj
TTY_MRES_OBJ = $(BUILD)/tty-mres.dobj
WCNSLS_MRES_OBJ = $(BUILD)/wcnsls-mres.dobj
OCNSLS_MRES_OBJ = $(BUILD)/ocnsls-mres.dobj
STORAGE_MRES_OBJ = $(BUILD)/storage-mres.dobj
SLV_MRES_OBJ = $(BUILD)/slv-mres.dobj
MRES_PACKAGE_OBJS = $(CTY_MRES_OBJ) $(CLK_MRES_OBJ) $(IO7_MRES_OBJ) $(DCS_MRES_OBJ) \
	$(GE_MRES_OBJ) $(DPY_MRES_OBJ) $(TTY_MRES_OBJ) $(WCNSLS_MRES_OBJ) $(OCNSLS_MRES_OBJ) $(STORAGE_MRES_OBJ) $(SLV_MRES_OBJ)

MINIT_TABLE_OBJ = $(BUILD)/module-table-test.dobj
DEVICE_TEST_ASM = $(KINIT_BUILD)/device-test-minit.s
DEVICE_TEST_OBJ = $(BUILD)/device-test-minit.dobj
END_OBJ = $(BUILD)/image-end-v1.dobj
IMAGE_DXR = $(BUILD)/kinit-v1.dxr
IMAGE_MAP = $(BUILD)/kinit-v1.map
IMAGE_RIM = $(BUILD)/kinit-v1.rim
IMAGE_RAW = $(BUILD)/kinit-v1.raw.words
IMAGE_WORDS = $(BUILD)/kinit-v1.words
USERLAND_BUILD_ROOT = $(BUILD)/userland-build
DSH_DXR = $(USERLAND_BUILD_ROOT)/userland-v1/dsh-v1.dxr
USERLAND_INPUTS = $(addprefix $(PROJECT_ROOT)/userland/, \
	v1/u_v1.c v1/u_v1.h v1/dsys_v1.h v1/crt0_v1.s v1/syscall_v1.s \
	v1/syscall_sentinel_v1.s exec_v1/commands_v1.c exec_v1/cmd_v1.h \
	dsh_v1/dsh.c Makefile) \
	$(PROJECT_ROOT)/system/kernel/syscall_v1.h
INITFS_ASM = $(BUILD)/initfs-v1.s
INITFS_OBJ = $(BUILD)/initfs-v1.dobj
STAGE1_DXR = $(BUILD)/stage1-dsk-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dsk-v1.pt
DISK_DIR = $(BUILD)/disk
DISK_IMAGE = $(DISK_DIR)/dsk0.dsk
SIMH_INI = $(BUILD)/kinit-v1.ini
SIMH_INI_NO_PTP = $(BUILD)/kinit-v1-no-ptp.ini
SIMH_INI_NO_SLV = $(BUILD)/kinit-v1-no-slv.ini
SIMH_INI_DPY = $(BUILD)/kinit-v1-dpy.ini
SIMH_INI_WCNSLS = $(BUILD)/kinit-v1-wcnsls.ini
SIMH_INI_DTC = $(BUILD)/kinit-v1-dtc.ini
SIMH_INI_MTC = $(BUILD)/kinit-v1-mtc.ini
LOG = $(BUILD)/kinit-v1.log
LOG_NO_PTP = $(BUILD)/kinit-v1-no-ptp.log
LOG_NO_SLV = $(BUILD)/kinit-v1-no-slv.log
LOG_DPY = $(BUILD)/kinit-v1-dpy.log
LOG_WCNSLS = $(BUILD)/kinit-v1-wcnsls.log
LOG_DTC = $(BUILD)/kinit-v1-dtc.log
DTC_TEST_WORDS = $(BUILD)/dtc-test-v1.words
DTC_TEST_TAPE = $(BUILD)/dtc-test-v1.dt
LOG_MTC = $(BUILD)/kinit-v1-mtc.log
MTC_TEST_WORDS = $(BUILD)/mtc-test-v1.words
MTC_TEST_TAPE = $(BUILD)/mtc-test-v1.tap
SIMH_LOG = $(BUILD)/kinit-v1.simh.log
SIMH_LOG_NO_PTP = $(BUILD)/kinit-v1-no-ptp.simh.log
SIMH_LOG_NO_SLV = $(BUILD)/kinit-v1-no-slv.simh.log
SIMH_LOG_DPY = $(BUILD)/kinit-v1-dpy.simh.log
SIMH_LOG_WCNSLS = $(BUILD)/kinit-v1-wcnsls.simh.log
SIMH_LOG_DTC = $(BUILD)/kinit-v1-dtc.simh.log
SIMH_LOG_MTC = $(BUILD)/kinit-v1-mtc.simh.log
DPY_DEBUG = $(BUILD)/kinit-v1-dpy.debug.log
WCNSLS_DEBUG = $(BUILD)/kinit-v1-wcnsls.debug.log
PTP_OUTPUT = $(BUILD)/ptp-output.bin
CR_INPUT = $(BUILD)/cr-input.txt
CP_OUTPUT = $(BUILD)/cp-output.txt

.DEFAULT_GOAL := image
.PHONY: image minboot test devicefs-v1-unit procfs-v1-unit proc-sched-v1-unit memfs-v1-unit file-v1-unit syscall-v1-unit clean


DEVICEFS_V1_UNIT = $(BUILD)/devicefs-v1-unit
$(DEVICEFS_V1_UNIT): devicefs-v1-unit.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.h \
    $(PROJECT_ROOT)/system/kernel/devicefs_v1.c \
    $(PROJECT_ROOT)/system/kernel/devicefs_v1.h | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -I$(PROJECT_ROOT)/system/kernel \
	    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/devicefs_v1.c devicefs-v1-unit.c -o $@

devicefs-v1-unit: $(DEVICEFS_V1_UNIT)
	$(DEVICEFS_V1_UNIT)

PROCFS_V1_UNIT = $(BUILD)/procfs-v1-unit
$(PROCFS_V1_UNIT): procfs-v1-unit.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.h \
    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/procfs_v1.h \
    $(PROJECT_ROOT)/system/kernel/proc_v1.c \
    $(PROJECT_ROOT)/system/kernel/proc_v1.h | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -I$(PROJECT_ROOT)/system/kernel \
	    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/proc_v1.c procfs-v1-unit.c -o $@

procfs-v1-unit: $(PROCFS_V1_UNIT)
	$(PROCFS_V1_UNIT)

PROC_SCHED_V1_UNIT = $(BUILD)/proc-sched-v1-unit
$(PROC_SCHED_V1_UNIT): proc-sched-v1-unit.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/proc_v1.c \
    $(PROJECT_ROOT)/system/kernel/sched_v1.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -I$(PROJECT_ROOT)/system/kernel \
	    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/proc_v1.c \
	    $(PROJECT_ROOT)/system/kernel/sched_v1.c proc-sched-v1-unit.c -o $@

proc-sched-v1-unit: $(PROC_SCHED_V1_UNIT)
	$(PROC_SCHED_V1_UNIT)

MEMFS_V1_UNIT = $(BUILD)/memfs-v1-unit
$(MEMFS_V1_UNIT): memfs-v1-unit.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/memfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/initfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/ramfs_v1.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -I$(PROJECT_ROOT)/system/kernel \
	    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/memfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/initfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/ramfs_v1.c memfs-v1-unit.c -o $@

memfs-v1-unit: $(MEMFS_V1_UNIT)
	$(MEMFS_V1_UNIT)

FILE_V1_UNIT = $(BUILD)/file-v1-unit
$(FILE_V1_UNIT): file-v1-unit.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/devicefs_v1.c \
    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/memfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/ramfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/file_v1.c \
    $(PROJECT_ROOT)/system/kernel/proc_v1.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -I$(PROJECT_ROOT)/system/kernel \
	    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/devicefs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/memfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/ramfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/file_v1.c \
	    $(PROJECT_ROOT)/system/kernel/proc_v1.c file-v1-unit.c -o $@

file-v1-unit: $(FILE_V1_UNIT)
	$(FILE_V1_UNIT)

SYSCALL_V1_UNIT = $(BUILD)/syscall-v1-unit
$(SYSCALL_V1_UNIT): syscall-v1-unit.c \
    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/devicefs_v1.c \
    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/memfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/ramfs_v1.c \
    $(PROJECT_ROOT)/system/kernel/file_v1.c \
    $(PROJECT_ROOT)/system/kernel/syscall_v1.c \
    $(PROJECT_ROOT)/system/kernel/proc_v1.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -Werror -I$(PROJECT_ROOT)/system/kernel \
	    $(PROJECT_ROOT)/system/kernel/vfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/devicefs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/procfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/memfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/ramfs_v1.c \
	    $(PROJECT_ROOT)/system/kernel/file_v1.c \
	    $(PROJECT_ROOT)/system/kernel/syscall_v1.c \
	    $(PROJECT_ROOT)/system/kernel/proc_v1.c syscall-v1-unit.c -o $@

syscall-v1-unit: $(SYSCALL_V1_UNIT)
	$(SYSCALL_V1_UNIT)

$(BUILD):
	mkdir -p $@

$(KINIT_ASM): $(KINIT_INPUTS) | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kinit BUILD='$(KINIT_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' KINIT_DEBUG='$(KINIT_DEBUG)'

$(KCORE_FIXED_ASM) $(MRES_ASM): $(KCORE_INPUTS) | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kcore BUILD='$(KCORE_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)'

$(BUILD)/%.dobj: $(KINIT_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/%.dobj: $(KCORE_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(DEVICE_TEST_ASM): device-test-minit.c $(PROJECT_ROOT)/system/kernel/kinit.h \
    $(PROJECT_ROOT)/system/kernel/kcore.h \
    $(PROJECT_ROOT)/system/kernel/kcore_pi.h \
    $(PROJECT_ROOT)/system/kernel/module.h $(PROJECT_ROOT)/system/kernel/card.h \
    $(PROJECT_ROOT)/system/kernel/dcs.h \
    $(PROJECT_ROOT)/system/kernel/ge.h \
    $(PROJECT_ROOT)/system/kernel/dpy.h \
    $(PROJECT_ROOT)/system/kernel/tty.h \
    $(PROJECT_ROOT)/system/kernel/wcnsls.h \
    $(PROJECT_ROOT)/system/kernel/ocnsls.h | $(BUILD)
	mkdir -p $(KINIT_BUILD)
	$(PDP10_GCC) -std=c99 -Os -I$(PROJECT_ROOT)/system/kernel -I$(PDP10_PREFIX)/include -S $< -o $@
$(DEVICE_TEST_OBJ): $(DEVICE_TEST_ASM) | $(BUILD)
	$(DAS) -c -o $@ $<
$(MINIT_TABLE_OBJ): module-table-test.s | $(BUILD)
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
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o60)[2:])' $$end`; \
	$(MKSTREAM) -i $(KCORE_RIM) -o $@ -b 060 -w "$$words"

$(CTY_MRES_OBJ): $(BUILD)/cty_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cty-mres.dxr -M $(BUILD)/cty-mres.map \
	    -R $@ -N cty_mres_package -X cty_pi_handler -X cty_put6 \
	    -X cty_putchar -X cty_getchar $(BUILD)/cty_io.dobj
$(CLK_MRES_OBJ): $(BUILD)/clk_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/clk-mres.dxr -M $(BUILD)/clk-mres.map \
	    -R $@ -N clk_mres_package -X clk_pi_handler -X clk_ticks -X clk_pi_service $(BUILD)/clk_io.dobj
$(IO7_MRES_OBJ): $(BUILD)/io7_io.dobj $(BUILD)/ptr_io.dobj $(BUILD)/ptp_io.dobj \
    $(BUILD)/cr_io.dobj $(BUILD)/cp_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/io7-mres.dxr -M $(BUILD)/io7-mres.map \
	    -R $@ -N io7_mres_package -X io7_pi_handler -X ptr_getchar \
	    -X ptp_putchar -X cr_read_card -X cp_punch_card \
	    $(BUILD)/io7_io.dobj $(BUILD)/ptr_io.dobj $(BUILD)/ptp_io.dobj \
	    $(BUILD)/cr_io.dobj $(BUILD)/cp_io.dobj
$(DCS_MRES_OBJ): $(BUILD)/dcs_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/dcs-mres.dxr -M $(BUILD)/dcs-mres.map \
	    -R $@ -N dcs_mres_package -X dcs_pi_handler -X dcs_getchar \
	    -X dcs_putchar $(BUILD)/dcs_io.dobj
$(GE_MRES_OBJ): $(BUILD)/ge_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ge-mres.dxr -M $(BUILD)/ge-mres.map \
	    -R $@ -N ge_mres_package -X ge_pi_handler -X ge_getchar \
	    -X ge_putchar $(BUILD)/ge_io.dobj
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
	    -R $@ -N wcnsls_mres_package -X wcnsls_read -X wcnsls_cono -X wcnsls_plot $(BUILD)/wcnsls_io.dobj
$(OCNSLS_MRES_OBJ): $(BUILD)/ocnsls_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ocnsls-mres.dxr -M $(BUILD)/ocnsls-mres.map \
	    -R $@ -N ocnsls_mres_package -X ocnsls_read $(BUILD)/ocnsls_io.dobj

$(STORAGE_MRES_OBJ): $(BUILD)/storage_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/storage-mres.dxr -M $(BUILD)/storage-mres.map \
	    -R $@ -N storage_mres_package -X storage_pi_handler -X dtc_read_words -X mtc_read_words \
	    -X dsk_read_sector -X dsk_write_sector -X mtc_write_words -X dtc_write_block \
	    -X storage_dct_handler $(BUILD)/storage_io.dobj

$(SLV_MRES_OBJ): $(BUILD)/slv_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/slv-mres.dxr -M $(BUILD)/slv-mres.map \
	    -R $@ -N slv_mres_package -X slv_pi_handler $(BUILD)/slv_io.dobj

$(KCORE_EMBED_ASM): kcore-embed-v1.s.in $(KCORE_MAP) | $(BUILD)
	end=`awk '$$1 == "__kcore_low_init_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o60)[2:])' $$end`; \
	sed "s/@KCORE_INIT_WORDS@/$$words/" $< > $@
$(KCORE_EMBED_OBJ): $(KCORE_EMBED_ASM)
	$(DAS) -c -o $@ $<

$(DSH_DXR): $(USERLAND_INPUTS)
	$(MAKE) -C $(PROJECT_ROOT)/userland build BUILD_ROOT='$(USERLAND_BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

$(INITFS_ASM): $(DSH_DXR)
	$(MKINITFS0) --label __initfs_v1_begin -o $@ \
	    /SYSTEM/INIT:$(DSH_DXR):555:dxr

$(INITFS_OBJ): $(INITFS_ASM)
	$(DAS) -c -o $@ $<

$(IMAGE_DXR): $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
    $(MRES_PACKAGE_OBJS) $(DEVICE_TEST_OBJ) $(INITFS_OBJ) $(END_OBJ) $(KCORE_MAP)
	$(DLINK) -b 040000 -A $(KCORE_MAP) -o $@ -M $(IMAGE_MAP) \
	    $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
	    $(MRES_PACKAGE_OBJS) $(DEVICE_TEST_OBJ) $(INITFS_OBJ) $(END_OBJ)
$(IMAGE_MAP): $(IMAGE_DXR)
	@test -f $@
$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 040000 $< $@
$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	end=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o40000)[2:])' $$end`; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 040000 -w "$$words"
$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(KCORE_STREAM) $(KCORE_MAP) mkimage-v1.py
	$(PYTHON) mkimage-v1.py --map $(IMAGE_MAP) --input $(IMAGE_RAW) \
	    --kcore $(KCORE_STREAM) --kcore-map $(KCORE_MAP) --output $@

STAGE1_SRC := $(PROJECT_ROOT)/system/stand/pdp6/dsk/stage1.s
$(STAGE1_DXR): $(STAGE1_SRC) | $(BUILD)
	$(DAS) --base-kernel -o $@ $<
$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b 060 $< $@
	printf '\252' >> $@
$(DISK_IMAGE): $(IMAGE_WORDS) $(STAGE1_TAPE)
	rm -rf $(DISK_DIR)
	$(MKDSK) -n 1 -m clean -p $(IMAGE_WORDS) -o $(DISK_DIR)
$(CR_INPUT): | $(BUILD)
	printf 'DAIMOS CARD TEST\n' > $@

$(SIMH_INI): simh-v1.ini.in $(STAGE1_TAPE) $(DISK_IMAGE) $(CR_INPUT)
	sed -e 's|@SIMH_LOG@|$(abspath $(SIMH_LOG))|g' \
	    -e 's|@STAGE1_TAPE@|$(abspath $(STAGE1_TAPE))|g' \
	    -e 's|@DISK_IMAGE@|$(abspath $(DISK_IMAGE))|g' \
	    -e 's|@PTP_OUTPUT@|$(abspath $(PTP_OUTPUT))|g' \
	    -e 's|@CR_INPUT@|$(abspath $(CR_INPUT))|g' \
	    -e 's|@CP_OUTPUT@|$(abspath $(CP_OUTPUT))|g' $< > $@
$(SIMH_INI_NO_PTP): $(SIMH_INI)
	sed -e 's/set ptp enabled/set ptp disabled/' -e '/attach -q -n ptp/d' $< > $@
$(SIMH_INI_NO_SLV): $(SIMH_INI)
	sed 's/set slave enabled/set slave disabled/' $< > $@
$(SIMH_INI_DPY): $(SIMH_INI)
	{ \
	    printf 'set debug -n %s\n' '$(abspath $(DPY_DEBUG))'; \
	    sed -e 's|$(abspath $(SIMH_LOG))|$(abspath $(SIMH_LOG_DPY))|' \
	        -e '/set ge disabled/a\
set dpy headless\
set dpy enabled\
set dpy debug=DATAIO' $<; \
	} > $@
$(SIMH_INI_WCNSLS): $(SIMH_INI)
	{ \
	    printf 'set debug -n %s\n' '$(abspath $(WCNSLS_DEBUG))'; \
	    sed -e 's|$(abspath $(SIMH_LOG))|$(abspath $(SIMH_LOG_WCNSLS))|' \
	        -e '/set ge disabled/a\
set wcnsls enabled\
set wcnsls debug=DATAIO\
set wcnsls debug=CONO' $<; \
	} > $@
$(DTC_TEST_WORDS): | $(BUILD)
	{ \
	    echo 012345670123; \
	    echo 076543210765; \
	    echo 000000000001; \
	    echo 0777777777776; \
	} > $@

$(DTC_TEST_TAPE): $(DTC_TEST_WORDS) $(MKDT)
	$(MKDT) -t dtc -p $(DTC_TEST_WORDS) -o $@

$(MTC_TEST_WORDS): | $(BUILD)
	{ \
	    echo 012345670123; \
	    echo 076543210765; \
	    echo 000000000001; \
	    echo 0777777777776; \
	} > $@

$(MTC_TEST_TAPE): $(MTC_TEST_WORDS) $(MKTAP)
	$(MKTAP) -t mtc -p $(MTC_TEST_WORDS) -o $@

$(SIMH_INI_DTC): $(SIMH_INI) $(DTC_TEST_TAPE)
	sed -e 's|$(abspath $(SIMH_LOG))|$(abspath $(SIMH_LOG_DTC))|' \
	    -e 's/set dtc disabled/set dtc enabled/' \
	    -e '/^go 020/i\
set dtc dct=04\
attach -q dtc0 $(abspath $(DTC_TEST_TAPE))' $< > $@
$(SIMH_INI_MTC): $(SIMH_INI) $(MTC_TEST_TAPE)
	sed -e 's|$(abspath $(SIMH_LOG))|$(abspath $(SIMH_LOG_MTC))|' \
	    -e 's/set mtc disabled/set mtc enabled/' \
	    -e '/^go 020/i\
set mtc dct=0\
set mtc0 7t\
attach -q mtc0 $(abspath $(MTC_TEST_TAPE))' $< > $@

image: $(SIMH_INI)

# Boot the minboot image on the terminal and leave DSH interactive.
# Type EXIT in DSH to return to the simulator and terminate.
minboot: image
	$(SIMH_PDP6) $(SIMH_INI)

test: image $(SIMH_INI_NO_PTP) $(SIMH_INI_NO_SLV) $(SIMH_INI_DPY) $(SIMH_INI_WCNSLS) $(SIMH_INI_DTC) $(SIMH_INI_MTC)
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI) $(SIMH_LOG) $(LOG) \
	    'DAIMON                              V0.1' \
	    'MACH                                PDP6' \
	    'MEM                                256 K' \
	    'CTY                                   OK' \
	    'HZ                               60 LINE' \
	    'PTR                                   OK' \
	    'PTP                                   OK' \
	    'CR                                    OK' \
	    'CP                                    OK' \
	    'DCS                               NO DEV' \
	    'GE                                NO DEV' \
	    'DPY                               NO DEV' \
	    'TTY                               LOADED' \
	    'WCNSLS                            LOADED' \
	    'OCNSLS                            LOADED' \
	    'DTC                               NO DEV' \
	    'MTC                               NO DEV' \
	    'DSK270                                OK' \
	    'SLV                                   OK' \
	    'HALT instruction' $(if $(filter 1,$(KINIT_DEBUG)),>/dev/null,)
ifeq ($(KINIT_DEBUG),1)
	@cat $(LOG)
endif
	@$(PYTHON) -c "from pathlib import Path; p=Path('$(PTP_OUTPUT)').read_bytes(); assert p == bytes([0o252]), p"
	@test -s $(CP_OUTPUT)
	@grep 'DAIMOS CARD TEST' $(CP_OUTPUT) >/dev/null
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_NO_PTP) $(SIMH_LOG_NO_PTP) $(LOG_NO_PTP) \
	    'PTP                               NO DEV' \
	    'HALT instruction' >/dev/null
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_NO_SLV) $(SIMH_LOG_NO_SLV) $(LOG_NO_SLV) \
	    'SLV                               NO DEV' \
	    'HALT instruction' >/dev/null
	@rm -f $(DPY_DEBUG)
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_DTC) $(SIMH_LOG_DTC) $(LOG_DTC) \
	    'DTC                                   OK' \
	    'MTC                               NO DEV' \
	    'HALT instruction' >/dev/null
	@$(PYTHON) dtc-write-check-v1.py --tape '$(DTC_TEST_TAPE)' --block 1
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_MTC) $(SIMH_LOG_MTC) $(LOG_MTC) \
	    'DTC                               NO DEV' \
	    'MTC                                   OK' \
	    'HALT instruction' >/dev/null
	@$(PYTHON) mtc-write-check-v1.py --tape '$(MTC_TEST_TAPE)' \
	    --record 1 --word 011223344556 --word 066554433221
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_DPY) $(SIMH_LOG_DPY) $(LOG_DPY) \
	    'DPY                                   OK' \
	    'HALT instruction' >/dev/null
	@$(PYTHON) dpy-banner-test-v1.py --log '$(DPY_DEBUG)' \
	    --version '$(PROJECT_ROOT)/VERSION'
	@rm -f $(WCNSLS_DEBUG)
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_WCNSLS) $(SIMH_LOG_WCNSLS) $(LOG_WCNSLS) \
	    'WCNSLS                            LOADED' 'HALT instruction' >/dev/null
	@$(PYTHON) wcnsls-banner-test-v1.py --debug-log '$(WCNSLS_DEBUG)' \
	    --simh-log '$(SIMH_LOG_WCNSLS)' --version '$(PROJECT_ROOT)/VERSION'
	@$(PYTHON) dcs-socket-test-v1.py --simh '$(SIMH_PDP6)' --ini '$(SIMH_INI)' --log '$(BUILD)/dcs-socket.log'
	@$(PYTHON) ge-shared-socket-test-v1.py --simh '$(SIMH_PDP6)' --ini '$(SIMH_INI)' --log '$(BUILD)/ge-shared-socket.log'
	@echo 'KINIT conditional-MRES device checkpoint test PASS'

clean:
	rm -rf $(BUILD)
