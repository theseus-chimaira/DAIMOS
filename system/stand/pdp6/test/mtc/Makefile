PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/mtc

include ../bin/stat.mk

BOOT_KIND = MTC
TEST_NAME = mtc-v1
STAGE1 = ../../mtc/stage1.s
STAGE1_COMMON_DEPS = ../../common/sequential-load.inc ../../common/validate-load.inc
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-mtc-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-mtc-v1.pt
PAYLOAD_TAPE = $(BUILD)/payload-v1.tap
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in
SIMH_SED_EXTRA =

$(PAYLOAD_TAPE): $(STAT_WORDS) $(MKTAP)
	$(MKTAP) -t mtc -p $(STAT_WORDS) -o $@

include ../bin/boot-test.mk
