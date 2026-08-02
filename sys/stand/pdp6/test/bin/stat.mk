STAND_ROOT := $(abspath $(CURDIR)/../..)
TEST_ROOT := $(STAND_ROOT)/test
TOOLS_DIR := $(TEST_ROOT)/tools
BIN_DIR := $(TEST_ROOT)/bin

BUILD ?= build
HOST_CC ?= cc
ifndef PDP10_PREFIX
$(error PDP10_PREFIX must name the directory containing the PDP-10 toolchain and emulator binaries)
endif

PDP10_AS ?= $(PDP10_PREFIX)/pdp10-dec-none-as
SIMH_PDP6 ?= $(PDP10_PREFIX)/pdp6
TIMEOUT ?= 30

STAT_INIT_WORDS ?= 01000
STAT_INIT_BASE ?= 072000
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
	$(HOST_CC) -std=c99 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkstream.c

$(BUILD)/words2pt: $(TOOLS_DIR)/words2pt.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/words2pt.c

$(BUILD)/mkdsk: $(TOOLS_DIR)/mkdsk.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkdsk.c

$(BUILD)/mktap: $(TOOLS_DIR)/mktap.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mktap.c

$(BUILD)/mkdt: $(TOOLS_DIR)/mkdt.c | $(BUILD)
	$(HOST_CC) -std=c99 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkdt.c

$(STAT_RIM): $(BIN_DIR)/stat.s | $(BUILD)
	$(PDP10_AS) --start $(STAT_INIT_BASE) $(BIN_DIR)/stat.s > $@

$(STAT_WORDS): $(STAT_RIM) $(BUILD)/mkstream
	$(BUILD)/mkstream -i $(STAT_RIM) -o $@ -b $(STAT_INIT_BASE) -w $(STAT_INIT_WORDS)

.PHONY: stat tools
stat: $(STAT_WORDS)
tools: $(TOOLS)
