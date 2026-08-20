PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/stand/pdp6/mtc

include ../bin/stat.mk

BOOT_KIND = MTC
TEST_NAME = mtc-v4
STAGE1 = $(PROJECT_ROOT)/system/stand/pdp6/mtc/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-mtc-v4.dxr
STAGE1_TAPE = $(BUILD)/stage1-mtc-v4.pt
LOWCORE_SRC = $(PROJECT_ROOT)/tests/system/stand/pdp6/ptr/lowcore-v1.s
LOWCORE_DXR = $(BUILD)/lowcore-v1.dxr
LOWCORE_RIM = $(BUILD)/lowcore-v1.rim
LOWCORE_STREAM = $(BUILD)/lowcore-v1.stream
LOWCORE_WORDS = $(BUILD)/lowcore-v1.words
RAW_PAYLOAD_WORDS = $(BUILD)/payload-v4.words
MTC_WORDS = $(BUILD)/mtc-v4.words
PAYLOAD_TAPE = $(BUILD)/payload-v4.tap
SIMH_INI = $(BUILD)/simh-v4.ini
SIMH_TEMPLATE = simh-v1.ini.in
SIMH_SED_EXTRA =

$(LOWCORE_DXR): $(LOWCORE_SRC) $(PROJECT_ROOT)/system/stand/pdp6/common/sixbit-output.inc | $(BUILD)
	$(DAS) --base-kernel -o $@ $(LOWCORE_SRC)

$(LOWCORE_RIM): $(LOWCORE_DXR)
	$(DXRCONVERT) --simh -b 060 $< $@

$(LOWCORE_STREAM): $(LOWCORE_RIM)
	$(MKSTREAM) -i $< -o $@ -b 060 -w 020

$(LOWCORE_WORDS): $(LOWCORE_STREAM)
	tail -n +3 $< > $@
	test `wc -l < $@` -eq 16

$(RAW_PAYLOAD_WORDS): $(STAT_WORDS) | $(BUILD)
	tail -n +3 $< > $@
	test `wc -l < $@` -eq $$(( $(STAT_INIT_WORDS) ))

$(MTC_WORDS): $(LOWCORE_WORDS) $(RAW_PAYLOAD_WORDS) | $(BUILD)
	cat $(LOWCORE_WORDS) $(RAW_PAYLOAD_WORDS) > $@

$(PAYLOAD_TAPE): $(MTC_WORDS) $(MKTAP)
	$(MKTAP) -t mtc -p $(MTC_WORDS) -o $@

include ../bin/boot-test.mk
