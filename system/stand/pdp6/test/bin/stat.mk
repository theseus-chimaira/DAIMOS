.DELETE_ON_ERROR:

STAND_ROOT := $(abspath $(CURDIR)/../..)
PROJECT_ROOT ?= $(abspath $(STAND_ROOT)/../../..)
TEST_ROOT := $(STAND_ROOT)/test
TOOLS_DIR := $(abspath $(STAND_ROOT)/../tools)
TOOLS_BUILD ?= $(PROJECT_ROOT)/build/system/stand/tools
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

TOOLS = \
	$(TOOLS_BUILD)/dxr2rim \
	$(TOOLS_BUILD)/mkdsk \
	$(TOOLS_BUILD)/mkdt \
	$(TOOLS_BUILD)/mkrim \
	$(TOOLS_BUILD)/mkstream \
	$(TOOLS_BUILD)/mktap \
	$(TOOLS_BUILD)/words2pt

DXR2RIM = $(TOOLS_BUILD)/dxr2rim
MKDSK = $(TOOLS_BUILD)/mkdsk
MKDT = $(TOOLS_BUILD)/mkdt
MKRIM = $(TOOLS_BUILD)/mkrim
MKSTREAM = $(TOOLS_BUILD)/mkstream
MKTAP = $(TOOLS_BUILD)/mktap
WORDS2PT = $(TOOLS_BUILD)/words2pt
TOOLS_STAMP = $(TOOLS_BUILD)/.tools-built

$(BUILD):
	mkdir -p $(BUILD)

$(STAT_BUILD):
	mkdir -p $(STAT_BUILD)

$(TOOLS_STAMP): $(TOOLS_DIR)/Makefile \
    $(TOOLS_DIR)/dxr2rim.c \
    $(TOOLS_DIR)/mkdsk.c \
    $(TOOLS_DIR)/mkdt.c \
    $(TOOLS_DIR)/mkrim.c \
    $(TOOLS_DIR)/mkstream.c \
    $(TOOLS_DIR)/mktap.c \
    $(TOOLS_DIR)/words2pt.c
	$(MAKE) -C $(TOOLS_DIR) build BUILD='$(TOOLS_BUILD)' HOST_CC='$(HOST_CC)'
	touch $@

$(TOOLS): $(TOOLS_STAMP)

$(STAT_DXR): $(BIN_DIR)/stat.s | $(STAT_BUILD)
	$(DAS) --base-kernel -o $@ $(BIN_DIR)/stat.s

$(STAT_RIM): $(STAT_DXR)
	$(DXRCONVERT) --simh -b $(STAT_INIT_BASE) $< $@

$(STAT_WORDS): $(STAT_RIM) $(MKSTREAM)
	$(MKSTREAM) -i $(STAT_RIM) -o $@ -b $(STAT_INIT_BASE) -w $(STAT_INIT_WORDS)

.PHONY: stat tools
stat: $(STAT_WORDS)
tools: $(TOOLS)
