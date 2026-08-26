PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD := $(BUILD_ROOT)/userland-v1
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
CPPFLAGS = -I$(PROJECT_ROOT)/system/kernel -I$(PROJECT_ROOT)/userland/v1 -I$(PROJECT_ROOT)/userland/exec_v1
CFLAGS = -std=c99 -Os

SRCS = v1/u_v1.c exec_v1/commands_v1.c exec_v1/standalone.c dsh_v1/dsh.c
ASMS = $(SRCS:%.c=$(BUILD)/%.s)

.DEFAULT_GOAL := check
.PHONY: check clean

check: $(ASMS)

$(BUILD)/%.s: %.c
	mkdir -p $(dir $@)
	$(PDP10_GCC) $(CFLAGS) $(CPPFLAGS) -S $< -o $@

clean:
	rm -rf $(BUILD)
