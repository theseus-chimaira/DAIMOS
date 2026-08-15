.DELETE_ON_ERROR:

STAND_ROOT := $(abspath $(CURDIR)/../..)
PROJECT_ROOT ?= $(abspath $(STAND_ROOT)/../../..)
TEST_ROOT := $(STAND_ROOT)/test
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
STAT_BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/bin
STAT_DXR = $(STAT_BUILD)/stat.dxr
STAT_RIM = $(STAT_BUILD)/stat.rim
STAT_WORDS = $(STAT_BUILD)/stat.words

MKDSK ?= $(PDP10_PREFIX)/bin/mkdsk
MKDT ?= $(PDP10_PREFIX)/bin/mkdt
MKSTREAM ?= $(PDP10_PREFIX)/bin/mkstream
MKTAP ?= $(PDP10_PREFIX)/bin/mktap
WORDS2PT ?= $(PDP10_PREFIX)/bin/words2pt

$(BUILD):
	mkdir -p $(BUILD)

$(STAT_BUILD):
	mkdir -p $(STAT_BUILD)

$(STAT_DXR): $(BIN_DIR)/stat.s | $(STAT_BUILD)
	$(DAS) --base-kernel -o $@ $(BIN_DIR)/stat.s

$(STAT_RIM): $(STAT_DXR)
	$(DXRCONVERT) --simh -b $(STAT_INIT_BASE) $< $@

$(STAT_WORDS): $(STAT_RIM) $(MKSTREAM)
	$(MKSTREAM) -i $(STAT_RIM) -o $@ -b $(STAT_INIT_BASE) -w $(STAT_INIT_WORDS)

.PHONY: stat
stat: $(STAT_WORDS)
