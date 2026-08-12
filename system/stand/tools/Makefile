.DEFAULT_GOAL := build

PROJECT_ROOT := $(abspath ../../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/system/stand/tools
HOST_CC ?= cc

TOOLS = \
	$(BUILD)/dxr2rim \
	$(BUILD)/mkdsk \
	$(BUILD)/mkdt \
	$(BUILD)/mkrim \
	$(BUILD)/mkstream \
	$(BUILD)/mktap \
	$(BUILD)/words2pt
TOOLS_STAMP = $(BUILD)/.tools-built

.PHONY: build test clean

build test: $(TOOLS_STAMP)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/dxr2rim: dxr2rim.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ dxr2rim.c

$(BUILD)/mkdsk: mkdsk.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ mkdsk.c

$(BUILD)/mkdt: mkdt.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ mkdt.c

$(BUILD)/mkrim: mkrim.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ mkrim.c

$(BUILD)/mkstream: mkstream.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ mkstream.c

$(BUILD)/mktap: mktap.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ mktap.c

$(BUILD)/words2pt: words2pt.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ words2pt.c

$(TOOLS_STAMP): $(TOOLS)
	touch $@

clean:
	rm -rf $(BUILD)
