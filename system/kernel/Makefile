PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
KINIT_DEBUG ?= 0
DCS_SIMH_COMPAT ?= 1
BUILD ?= $(BUILD_ROOT)/system/kernel$(if $(filter 1,$(KINIT_DEBUG)),-debug,)
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))

ifneq ($(filter $(DCS_SIMH_COMPAT),0 1),$(DCS_SIMH_COMPAT))
$(error DCS_SIMH_COMPAT must be 0 or 1)
endif

KINIT_SRCS = kinit.c kinit_diag.c module_minit.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(BUILD)/kinit_io.s $(BUILD)/module_io.s
KINIT_CPPFLAGS = -DDAIMON_VERSION_TEXT='"V$(DAIMON_VERSION)  "'
ifeq ($(KINIT_DEBUG),1)
KINIT_CPPFLAGS += -DKINIT_DEBUG
endif

KCORE_FIXED_ASM = $(BUILD)/kcore_entry.s $(BUILD)/kcore_io.s \
	$(BUILD)/kcore_end.s

MRES_SRCS =
MRES_C_ASM = $(MRES_SRCS:%.c=$(BUILD)/%.s)
MRES_ASM = $(MRES_C_ASM) $(BUILD)/cty_io.s $(BUILD)/clk_io.s \
	$(BUILD)/ptr_io.s $(BUILD)/ptp_io.s $(BUILD)/cr_io.s \
	$(BUILD)/cp_io.s $(BUILD)/dcs_io.s $(BUILD)/wcnsls_io.s $(BUILD)/ocnsls_io.s

.DEFAULT_GOAL := kinit
.PHONY: kinit kcore test clean

kinit: $(KINIT_ASM)
kcore: $(KCORE_FIXED_ASM) $(MRES_ASM)

test:
	$(MAKE) -C $(PROJECT_ROOT)/tests/system/kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel' KINIT_DEBUG='$(KINIT_DEBUG)'

$(BUILD)/kinit.s $(BUILD)/kinit_diag.s $(BUILD)/module_minit.s: $(BUILD)/%.s: %.c kinit.h module.h mres.h kcore.h kcore_pi.h cty.h clk.h pt.h card.h dcs.h wcnsls.h ocnsls.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/dcs_io.s: dcs_io.s FORCE_DCS_CONFIG
	mkdir -p $(BUILD)
	@{ \
		if [ '$(DCS_SIMH_COMPAT)' = 1 ]; then \
			printf '.set DCS_SIMH_COMPAT,1\n'; \
		fi; \
		cat $<; \
	} > $@

.PHONY: FORCE_DCS_CONFIG
FORCE_DCS_CONFIG:

$(BUILD)/%.s: %.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
