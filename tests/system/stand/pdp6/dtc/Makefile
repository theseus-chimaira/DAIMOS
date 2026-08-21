PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/stand/pdp6/dtc

include ../bin/stat.mk

BOOT_KIND = DTC
TEST_NAME = dtc-v3
STAGE1 = $(PROJECT_ROOT)/system/stand/pdp6/dtc/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-dtc-v3.dxr
STAGE1_TAPE = $(BUILD)/stage1-dtc-v3.pt
DTC_WORDS = $(BUILD)/dtc-v3.words
PAYLOAD_TAPE = $(BUILD)/payload-v3.dt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in
SIMH_SED_EXTRA =

$(DTC_WORDS): $(STAT_WORDS) | $(BUILD)
	cp $(STAT_WORDS) $@

$(PAYLOAD_TAPE): $(DTC_WORDS) $(MKDT)
	$(MKDT) -t dtc -p $(DTC_WORDS) -o $@

include ../bin/boot-test.mk
