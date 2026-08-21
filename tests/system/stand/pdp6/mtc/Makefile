PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/stand/pdp6/mtc

include ../bin/stat.mk

BOOT_KIND = MTC
TEST_NAME = mtc-v4
STAGE1 = $(PROJECT_ROOT)/system/stand/pdp6/mtc/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-mtc-v4.dxr
STAGE1_TAPE = $(BUILD)/stage1-mtc-v4.pt
RAW_PAYLOAD_WORDS = $(BUILD)/payload-v4.words
MTC_WORDS = $(BUILD)/mtc-v4.words
PAYLOAD_TAPE = $(BUILD)/payload-v4.tap
SIMH_INI = $(BUILD)/simh-v4.ini
SIMH_TEMPLATE = simh-v1.ini.in
SIMH_SED_EXTRA =

$(RAW_PAYLOAD_WORDS): $(STAT_WORDS) | $(BUILD)
	tail -n +3 $< > $@
	test `wc -l < $@` -eq $$(( $(STAT_INIT_WORDS) ))

$(MTC_WORDS): $(RAW_PAYLOAD_WORDS) | $(BUILD)
	cp $(RAW_PAYLOAD_WORDS) $@

$(PAYLOAD_TAPE): $(MTC_WORDS) $(MKTAP)
	$(MKTAP) -t mtc -p $(MTC_WORDS) -o $@

include ../bin/boot-test.mk
