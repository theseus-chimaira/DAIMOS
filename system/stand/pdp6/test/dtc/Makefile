PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/dtc

include ../bin/stat.mk

BOOT_KIND = DTC
TEST_NAME = dtc-v1
STAGE1 = ../../dtc/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-dtc-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dtc-v1.pt
PAYLOAD_TAPE = $(BUILD)/payload-v1.dt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in
SIMH_SED_EXTRA =

$(PAYLOAD_TAPE): $(STAT_WORDS) $(MKDT)
	$(MKDT) -t dtc -p $(STAT_WORDS) -o $@

include ../bin/boot-test.mk
