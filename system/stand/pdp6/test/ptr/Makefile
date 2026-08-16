PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/ptr

include ../bin/stat.mk

BOOT_KIND = PTR
TEST_NAME = ptr-v1
STAGE1 = ../../ptr/stage1.s
STAGE1_COMMON_DEPS = ../../common/sequential-load.inc ../../common/validate-load.inc
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-ptr-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-ptr-v1.pt
PAYLOAD_TAPE = $(BUILD)/payload-v1.pt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in
SIMH_SED_EXTRA = -e 's|@STAGE1_ENTRY@|$(STAGE1_ENTRY)|g'

$(PAYLOAD_TAPE): $(STAT_WORDS) $(WORDS2PT)
	$(WORDS2PT) -t ptr -p $(STAT_WORDS) -o $@

include ../bin/boot-test.mk
