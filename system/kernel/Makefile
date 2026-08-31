PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
KINIT_DEBUG ?= 0
BUILD ?= $(BUILD_ROOT)/system/kernel$(if $(filter 1,$(KINIT_DEBUG)),-debug,)
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))


KINIT_SRCS = kinit.c kinit_diag.c module_minit.c kfs_boot_v2.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(BUILD)/kinit_io.s $(BUILD)/module_io.s
KINIT_CPPFLAGS = -DDAIMON_VERSION_TEXT='"V$(DAIMON_VERSION)  "'
ifeq ($(KINIT_DEBUG),1)
KINIT_CPPFLAGS += -DKINIT_DEBUG
endif

KCORE_C_SRCS = kboot_v1.c exec_v1.c vfs_v1.c devicefs_v1.c \
	memfs_v1.c dtfs_v1.c dsk270.c d6fs_v2.c d6fs_provider_v2.c d6fs_disk_v2.c d6log_v2.c file_v1.c
KCORE_C_ASM = $(KCORE_C_SRCS:%.c=$(BUILD)/%.s)
KCORE_FIXED_ASM = $(BUILD)/kcore_entry.s $(KCORE_C_ASM) \
	$(BUILD)/ret_pdp10.s $(BUILD)/dtfs_pdp10.s $(BUILD)/dsk270_pdp10.s $(BUILD)/kfmt_pdp10.s $(BUILD)/vfs_pdp10.s $(BUILD)/memfs_pdp10.s $(BUILD)/file_pdp10.s $(BUILD)/proc_pdp10.s $(BUILD)/device_state_pdp10.s $(BUILD)/procfs_pdp10.s $(BUILD)/devicefs_pdp10.s $(BUILD)/syscall_info_pdp10.s $(BUILD)/syscall_dispatch_v2.s $(BUILD)/mach_user_v1.s $(BUILD)/kcore_io.s $(BUILD)/kcore_end.s

MRES_SRCS =
MRES_C_ASM = $(MRES_SRCS:%.c=$(BUILD)/%.s)
MRES_ASM = $(MRES_C_ASM) $(BUILD)/cty_io.s $(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s $(BUILD)/ptp_io.s $(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s $(BUILD)/io7_io.s $(BUILD)/dcs_io.s $(BUILD)/ge_io.s $(BUILD)/dpy_io.s \
	$(BUILD)/tty_io.s $(BUILD)/wcnsls_io.s $(BUILD)/ocnsls_io.s $(BUILD)/storage_io.s $(BUILD)/slv_io.s

.DEFAULT_GOAL := kinit
.PHONY: kinit kcore clean

kinit: $(KINIT_ASM)
kcore: $(KCORE_FIXED_ASM) $(MRES_ASM)

$(BUILD)/kinit.s $(BUILD)/kinit_diag.s $(BUILD)/module_minit.s $(BUILD)/kfs_boot_v2.s: $(BUILD)/%.s: %.c kinit.h module.h mres.h kcore.h kboot_v1.h vfs_v1.h memfs_v1.h initfs_v1.h exec_v1.h proc_v1.h mach_user_v1.h kcore_pi.h cty.h clk.h pt.h card.h dcs.h ge.h dpy.h tty.h wcnsls.h ocnsls.h storage.h slv.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(KCORE_C_ASM): $(BUILD)/%.s: %.c kcore.h kboot_v1.h vfs_v1.h kfmt_v1.h devicefs_v1.h procfs_v1.h memfs_v1.h dtfs_v1.h dsk270.h d6fs_v2.h d6fs_provider_v2.h d6fs_disk_v2.h d6log_v2.h initfs_v1.h file_v1.h proc_v1.h syscall_v1.h exec_v1.h proc_v1.h mach_user_v1.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/%.s: %.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
