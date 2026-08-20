PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/system/kernel
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc

KINIT_SRCS = \
	kinit.c \
	kinit_boot.c \
	kinit_diag.c \
	kinit_reloc.c
KINIT_ASM = $(KINIT_SRCS:%.c=$(BUILD)/%.s)

.DEFAULT_GOAL := kinit

.PHONY: kinit clean

kinit: $(KINIT_ASM)

$(BUILD)/%.s: %.c kinit.h
	mkdir -p $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I. -I$(PROJECT_ROOT)/system/headers -S $< -o $@

clean:
	rm -rf $(BUILD)
