PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
KINIT_DEBUG ?= 0
BUILD ?= $(BUILD_ROOT)/system/kernel$(if $(filter 1,$(KINIT_DEBUG)),-debug,)
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))

KINIT_SRCS = \
	kinit.c \
	kinit_diag.c \
	module_minit.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(BUILD)/kinit_io.s
KINIT_CPPFLAGS = -DDAIMON_VERSION_TEXT='"V$(DAIMON_VERSION)  "'
ifeq ($(KINIT_DEBUG),1)
KINIT_CPPFLAGS += -DKINIT_DEBUG
endif
KCORE_CPPFLAGS =

KCORE_SRCS = \
	kcore.c \
	kcore_pi.c \
	cty.c \
	clk.c \
	pt.c \
	card.c
KCORE_C_ASM = $(KCORE_SRCS:%.c=$(BUILD)/%.s)
KCORE_ASM = $(KCORE_C_ASM) $(BUILD)/kcore_entry.s $(BUILD)/kcore_io.s \
	$(BUILD)/cty_io.s $(BUILD)/clk_io.s $(BUILD)/pt_io.s $(BUILD)/card_io.s $(BUILD)/joy_io.s $(BUILD)/resident_end.s

.DEFAULT_GOAL := kinit

.PHONY: kinit kcore test clean

kinit: $(KINIT_ASM)

kcore: $(KCORE_ASM)

test:
	$(MAKE) -C $(PROJECT_ROOT)/tests/system/kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel' KINIT_DEBUG='$(KINIT_DEBUG)'

$(BUILD)/%.s: %.c kinit.h module.h mres.h cty.h clk.h pt.h card.h joy.h kcore_pi.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/kinit_io.s: kinit_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/kcore_%.s: kcore_%.c kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KCORE_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/kcore.s: kcore.c kcore.h kcore_pi.h clk.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KCORE_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@


$(BUILD)/cty.s: cty.c cty.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KCORE_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/clk.s: clk.c clk.h kcore.h kcore_pi.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KCORE_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/pt.s: pt.c pt.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KCORE_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/card.s: card.c card.h kcore.h kcore_pi.h kcore_io.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KCORE_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/kcore_io.s: kcore_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/cty_io.s: cty_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/clk_io.s: clk_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/pt_io.s: pt_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/card_io.s: card_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/joy_io.s: joy_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/kcore_entry.s: kcore_entry.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/resident_end.s: resident_end.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
