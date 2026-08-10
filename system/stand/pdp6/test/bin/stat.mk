.DELETE_ON_ERROR:

STAND_ROOT := $(abspath $(CURDIR)/../..)
TEST_ROOT := $(STAND_ROOT)/test
TOOLS_DIR := $(TEST_ROOT)/tools
BIN_DIR := $(TEST_ROOT)/bin

BUILD ?= build
HOST_CC ?= cc
PDP10_PREFIX ?= $(HOME)/cross

DAS ?= $(PDP10_PREFIX)/bin/das
DXRCONVERT ?= $(PDP10_PREFIX)/bin/dxrconvert
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
TIMEOUT ?= 120

STAT_INIT_WORDS ?= 01000
STAT_INIT_BASE ?= 040000
STAT_DXR = $(BUILD)/stat.dxr
STAT_RIM = $(BUILD)/stat.rim
STAT_WORDS = $(BUILD)/stat.words

TOOLS = \
	$(BUILD)/mkstream \
	$(BUILD)/words2pt \
	$(BUILD)/mkdsk \
	$(BUILD)/mktap \
	$(BUILD)/mkdt

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/mkstream: $(TOOLS_DIR)/mkstream.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkstream.c

$(BUILD)/words2pt: $(TOOLS_DIR)/words2pt.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/words2pt.c

$(BUILD)/mkdsk: $(TOOLS_DIR)/mkdsk.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkdsk.c

$(BUILD)/mktap: $(TOOLS_DIR)/mktap.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mktap.c

$(BUILD)/mkdt: $(TOOLS_DIR)/mkdt.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkdt.c

$(STAT_DXR): $(BIN_DIR)/stat.s | $(BUILD)
	$(DAS) --base-kernel -o $@ $(BIN_DIR)/stat.s

$(STAT_RIM): $(STAT_DXR)
	$(DXRCONVERT) --simh -b $(STAT_INIT_BASE) $< $@

$(STAT_WORDS): $(STAT_RIM) $(BUILD)/mkstream
	$(BUILD)/mkstream -i $(STAT_RIM) -o $@ -b $(STAT_INIT_BASE) -w $(STAT_INIT_WORDS)

.PHONY: stat tools
stat: $(STAT_WORDS)
tools: $(TOOLS)
