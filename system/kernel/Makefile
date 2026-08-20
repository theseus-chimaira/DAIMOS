PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/system/kernel
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
VERSION_FILE := $(PROJECT_ROOT)/VERSION
DAIMON_VERSION := $(strip $(shell cat $(VERSION_FILE)))
DAIMON_VERSION_MAJOR := $(word 1,$(subst ., ,$(DAIMON_VERSION)))
DAIMON_VERSION_MINOR := $(word 2,$(subst ., ,$(DAIMON_VERSION)))

KINIT_SRCS = \
	kinit.c \
	kinit_boot.c \
	kinit_diag.c \
	kinit_reloc.c
KINIT_C_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)
KINIT_ASM = $(KINIT_C_ASM) $(BUILD)/kinit_io.s
KINIT_CPPFLAGS = \
	-DDAIMON_VERSION_MAJOR=$(DAIMON_VERSION_MAJOR) \
	-DDAIMON_VERSION_MINOR=$(DAIMON_VERSION_MINOR)

.DEFAULT_GOAL := kinit

.PHONY: kinit test clean

kinit: $(KINIT_ASM)

test:
	$(MAKE) -C $(PROJECT_ROOT)/tests/system/kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel'

$(BUILD)/%.s: %.c kinit.h $(VERSION_FILE)
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os $(KINIT_CPPFLAGS) -I. -S $< -o $@

$(BUILD)/kinit_io.s: kinit_io.s
	mkdir -p $(BUILD)
	cp $< $@

clean:
	rm -rf $(BUILD)
