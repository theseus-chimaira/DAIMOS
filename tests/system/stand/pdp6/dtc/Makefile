PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/stand/pdp6/dtc

include ../bin/stat.mk

BOOT_KIND = DTC
TEST_NAME = dtc-v3
STAGE1 = $(PROJECT_ROOT)/system/stand/pdp6/dtc/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-dtc-v3.dxr
STAGE1_TAPE = $(BUILD)/stage1-dtc-v3.pt
LOWCORE_SRC = $(PROJECT_ROOT)/tests/system/stand/pdp6/ptr/lowcore-v1.s
LOWCORE_DXR = $(BUILD)/lowcore-v1.dxr
LOWCORE_RIM = $(BUILD)/lowcore-v1.rim
LOWCORE_STREAM = $(BUILD)/lowcore-v1.stream
LOWCORE_WORDS = $(BUILD)/lowcore-v1.words
DTC_WORDS = $(BUILD)/dtc-v3.words
PAYLOAD_TAPE = $(BUILD)/payload-v3.dt
SIMH_INI = $(BUILD)/simh-v1.ini
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

$(DTC_WORDS): $(LOWCORE_WORDS) $(STAT_WORDS) | $(BUILD)
	cat $(LOWCORE_WORDS) $(STAT_WORDS) > $@

$(PAYLOAD_TAPE): $(DTC_WORDS) $(MKDT)
	$(MKDT) -t dtc -p $(DTC_WORDS) -o $@

include ../bin/boot-test.mk
