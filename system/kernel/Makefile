PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
KINIT_DEBUG ?= 0
BUILD ?= $(BUILD_ROOT)/system/kernel$(if $(filter 1,$(KINIT_DEBUG)),-debug,)
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))

KINIT_SRCS = kinit.c kinit_diag.c module_minit.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(BUILD)/kinit_io.s $(BUILD)/module_io.s
KINIT_CPPFLAGS = -DDAIMON_VERSION_TEXT='"V$(DAIMON_VERSION)  "'
ifeq ($(KINIT_DEBUG),1)
KINIT_CPPFLAGS += -DKINIT_DEBUG
endif

KCORE_FIXED_SRCS = kcore.c kcore_pi.c
KCORE_FIXED_C_ASM = $(KCORE_FIXED_SRCS:%.c=$(BUILD)/%.s)
KCORE_FIXED_ASM = $(KCORE_FIXED_C_ASM) $(BUILD)/kcore_entry.s \
	$(BUILD)/kcore_io.s $(BUILD)/kcore_end.s

MRES_SRCS = cty.c clk.c ptr.c ptp.c cr.c cp.c
MRES_C_ASM = $(MRES_SRCS:%.c=$(BUILD)/%.s)
MRES_ASM = $(MRES_C_ASM) $(BUILD)/cty_io.s $(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s $(BUILD)/ptp_io.s $(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s $(BUILD)/wcnsls_io.s

.DEFAULT_GOAL := kinit
.PHONY: kinit kcore test clean

kinit: $(KINIT_ASM)
kcore: $(KCORE_FIXED_ASM) $(MRES_ASM)

test:
	$(MAKE) -C $(PROJECT_ROOT)/tests/system/kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel' KINIT_DEBUG='$(KINIT_DEBUG)'

$(BUILD)/kinit.s $(BUILD)/kinit_diag.s $(BUILD)/module_minit.s: $(BUILD)/%.s: %.c kinit.h module.h mres.h kcore.h kcore_pi.h kcore_io.h cty.h clk.h pt.h card.h wcnsls.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/kcore.s $(BUILD)/kcore_pi.s: $(BUILD)/%.s: %.c kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/cty.s: cty.c cty.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@
$(BUILD)/clk.s: clk.c clk.h kcore.h kcore_pi.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@
$(BUILD)/ptr.s: ptr.c pt.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@
$(BUILD)/ptp.s: ptp.c pt.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@
$(BUILD)/cr.s: cr.c card.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@
$(BUILD)/cp.s: cp.c card.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/%.s: %.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
