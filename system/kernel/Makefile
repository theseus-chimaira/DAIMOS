PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
KINIT_DEBUG ?= 0
DTFS_TENEX ?= 0
DTFS_ITS ?= 0
PROC_BOOT_USERS ?= 1
DTFS_FOREIGN := $(if $(filter 1,$(DTFS_TENEX) $(DTFS_ITS)),1,0)
BUILD ?= $(BUILD_ROOT)/system/kernel$(if $(filter 1,$(KINIT_DEBUG)),-debug,)
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
MKBOOTBANNER ?= $(PDP10_PREFIX)/bin/mkbootbanner
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))


KINIT_SRCS = kinit.c kinit_diag.c module_minit.c kfs_boot.c kinit_boot.c d6fs_boot.c d6fs_super_boot.c diskset_boot.c logstore.c proc_boot.c proc_swap_boot.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_LATE_SRCS = exec.c proc_late.c proc_swap_late.c mm_boot.c kinit_late.c
KINIT_LATE_C_ASM = $(KINIT_LATE_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(KINIT_LATE_C_ASM) $(BUILD)/kinit_io.s $(BUILD)/module_io.s $(BUILD)/bootbanner_generated.s \
	$(BUILD)/kinit_late_begin.s $(BUILD)/kinit_late_end.s
KINIT_CPPFLAGS = -DDAIMON_VERSION_TEXT='"V$(DAIMON_VERSION)  "' -DPROC_BOOT_USERS=$(PROC_BOOT_USERS)
ifeq ($(KINIT_DEBUG),1)
KINIT_CPPFLAGS += -DKINIT_DEBUG
endif

KCORE_C_SRCS = vfs.c devicefs.c procfs.c mm.c module_runtime.c proc.c proc_swap.c \
	file.c
KCORE_C_ASM = $(KCORE_C_SRCS:%.c=$(BUILD)/%.s)
KCORE_FIXED_ASM = $(KCORE_C_ASM) \
	$(BUILD)/ret_pdp10.s $(BUILD)/dsk270_pdp10.s $(BUILD)/kfmt_pdp10.s $(BUILD)/vfs_pdp10.s $(BUILD)/fs_mres_pdp10.s $(BUILD)/file_pdp10.s $(BUILD)/proc_pdp10.s $(BUILD)/device_state_pdp10.s $(BUILD)/devicefs_pdp10.s $(BUILD)/syscall_info_pdp10.s $(BUILD)/syscall_dispatch.s $(BUILD)/mach_user.s $(BUILD)/mach_enter_user.s $(BUILD)/kcore_io.s $(BUILD)/storage_router.s $(BUILD)/kcore_end.s

MRES_SRCS = d6fs.c d6fs_provider.c
MRES_C_ASM = $(MRES_SRCS:%.c=$(BUILD)/%.s)
MRES_ASM = $(BUILD)/dtfs.s $(MRES_C_ASM) $(BUILD)/cty_io.s $(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s $(BUILD)/ptp_io.s $(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s $(BUILD)/diskset_dispatch.s $(BUILD)/dcs_io.s $(BUILD)/ge_io.s $(BUILD)/dpy_io.s \
	$(BUILD)/tty_io.s $(BUILD)/dtfs_pdp10.s $(BUILD)/memfs_pdp10.s $(BUILD)/d6fs_pdp10.s $(BUILD)/wcnsls_io.s $(BUILD)/ocnsls_io.s $(BUILD)/dsk_io.s $(BUILD)/tape_io.s $(BUILD)/slv_io.s

.DEFAULT_GOAL := kinit
.PHONY: kinit kcore clean

kinit: $(KINIT_ASM)
kcore: $(KCORE_FIXED_ASM) $(MRES_ASM)

$(KINIT_C_ASM) $(KINIT_LATE_C_ASM): $(BUILD)/%.s: %.c kinit.h module.h mres.h kcore.h kboot.h vfs.h devicefs.h memfs.h initfs.h exec.h proc.h mach_user.h fs_mres.h mm.h mm_internal.h kcore_pi.h cty.h clk.h pt.h card.h dcs.h ge.h dpy.h tty.h wcnsls.h ocnsls.h storage.h slv.h d6fs_boot.h logstore.h d6fs_provider.h dsk270.h diskset_boot.h diskset_mres.h module_runtime.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -S $< -o $@

$(BUILD)/bootbanner_generated.s: $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(MKBOOTBANNER) -v '$(DAIMON_VERSION)' -o $@

$(KCORE_C_ASM): $(BUILD)/%.s: %.c kcore.h kboot.h vfs.h kfmt.h devicefs.h procfs.h memfs.h dtfs.h dsk270.h d6fs.h d6fs_provider.h d6fs_boot.h logstore.h initfs.h file.h proc.h syscall.h exec.h proc.h mach_user.h fs_mres.h mm.h mm_internal.h module_runtime.h proc_swap.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -DPROC_BOOT_USERS=$(PROC_BOOT_USERS) -I. -S $< -o $@

$(MRES_C_ASM): $(BUILD)/%.s: %.c kcore.h vfs.h fs_mres.h memfs.h dtfs.h d6fs.h d6fs_provider.h diskset.h diskset_mres.h mres.h dsk270.h file.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -S $< -o $@

$(BUILD)/dtfs.s: dtfs.c kcore.h vfs.h fs_mres.h memfs.h dtfs.h d6fs.h d6fs_provider.h diskset.h diskset_mres.h mres.h dsk270.h file.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -DDTFS_ENABLE_TENEX=$(DTFS_TENEX) -DDTFS_ENABLE_ITS=$(DTFS_ITS) -I. -S $< -o $@

$(BUILD)/dtfs_pdp10.s: dtfs_pdp10.s
	mkdir -p $(BUILD)
	{ echo ".set DTFS_ENABLE_TENEX,$(DTFS_TENEX)"; echo ".set DTFS_ENABLE_ITS,$(DTFS_ITS)"; echo ".set DTFS_ENABLE_FOREIGN,$(DTFS_FOREIGN)"; cat $<; } > $@

$(BUILD)/%.s: %.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
