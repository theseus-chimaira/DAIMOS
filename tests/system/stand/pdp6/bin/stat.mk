.DELETE_ON_ERROR:

PROJECT_ROOT ?= $(abspath $(CURDIR)/../../../../..)
TEST_ROOT := $(PROJECT_ROOT)/tests/system/stand/pdp6
BIN_DIR := $(TEST_ROOT)/bin

BUILD ?= build
HOST_CC ?= cc
PDP10_PREFIX ?= $(HOME)/cross
PDP10_TOOL_PREFIXES := $(strip $(PDP10_PREFIx) $(PDP10_PREFIX))

ifeq ($(strip $(DAS)),)
DAS := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/das)) $(shell command -v das 2>/dev/null))
endif
ifeq ($(strip $(DXRCONVERT)),)
DXRCONVERT := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/dxrconvert)) $(shell command -v dxrconvert 2>/dev/null))
endif
ifeq ($(strip $(SIMH_PDP6)),)
SIMH_PDP6 := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/pdp6)) $(shell command -v pdp6 2>/dev/null))
endif

TIMEOUT ?= 120

STAT_INIT_WORDS ?= 01000
STAT_INIT_BASE ?= 040000
STAT_BUILD ?= $(abspath $(BUILD)/../bin)
STAT_DXR = $(STAT_BUILD)/stat.dxr
STAT_RIM = $(STAT_BUILD)/stat.rim
STAT_WORDS = $(STAT_BUILD)/stat.words

ifeq ($(strip $(MKDSK)),)
MKDSK := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/mkdsk)) $(shell command -v mkdsk 2>/dev/null))
endif
ifeq ($(strip $(MKDT)),)
MKDT := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/mkdt)) $(shell command -v mkdt 2>/dev/null))
endif
ifeq ($(strip $(MKSTREAM)),)
MKSTREAM := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/mkstream)) $(shell command -v mkstream 2>/dev/null))
endif
ifeq ($(strip $(MKTAP)),)
MKTAP := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/mktap)) $(shell command -v mktap 2>/dev/null))
endif
ifeq ($(strip $(WORDS2PT)),)
WORDS2PT := $(firstword $(foreach p,$(PDP10_TOOL_PREFIXES),$(wildcard $(p)/bin/words2pt)) $(shell command -v words2pt 2>/dev/null))
endif

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
