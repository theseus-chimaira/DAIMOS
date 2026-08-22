PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/system/kernel
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))

KINIT_SRCS = \
	kinit.c \
	kinit_diag.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(BUILD)/kinit_io.s
KINIT_CPPFLAGS = -DDAIMON_VERSION_TEXT='"V$(DAIMON_VERSION)  "'
KINIT_DEBUG ?= 0
ifeq ($(KINIT_DEBUG),1)
KINIT_CPPFLAGS += -DKINIT_DEBUG
endif

KCORE_SRCS = \
	kcore.c
KCORE_C_ASM = $(KCORE_SRCS:%.c=$(BUILD)/%.s)
KCORE_ASM = $(KCORE_C_ASM) $(BUILD)/kcore_entry.s $(BUILD)/resident_end.s

.DEFAULT_GOAL := kinit

.PHONY: kinit kcore test clean

kinit: $(KINIT_ASM)

kcore: $(KCORE_ASM)

test:
	$(MAKE) -C $(PROJECT_ROOT)/tests/system/kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel'

$(BUILD)/%.s: %.c kinit.h module.h mres.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -I$(PDP10_PREFIX)/include -S $< -o $@

$(BUILD)/kinit_io.s: kinit_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/kcore_%.s: kcore_%.c kcore.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -S $< -o $@

$(BUILD)/kcore.s: kcore.c kcore.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -S $< -o $@

$(BUILD)/kcore_entry.s: kcore_entry.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/kcore_io.s: kcore_io.s
	mkdir -p $(BUILD)
	cp $< $@

$(BUILD)/resident_end.s: resident_end.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
