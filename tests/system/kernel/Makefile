PROJECT_ROOT := $(abspath ../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/kernel
KINIT_DEBUG ?= 0
KINIT_VARIANT := $(if $(filter 1,$(KINIT_DEBUG)),debug,normal)
KINIT_BUILD := $(BUILD)/kinit-asm-$(KINIT_VARIANT)
KCORE_BUILD := $(BUILD)/kcore-asm
PDP10_PREFIX ?= $(HOME)/cross

DAS ?= $(PDP10_PREFIX)/bin/das
DLINK ?= $(PDP10_PREFIX)/bin/dlink
DXRCONVERT ?= $(PDP10_PREFIX)/bin/dxrconvert
MKSTREAM ?= $(PDP10_PREFIX)/bin/mkstream
MKDSK ?= $(PDP10_PREFIX)/bin/mkdsk
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
PYTHON ?= python3
PDP10_GCC ?= $(PDP10_PREFIX)/bin/pdp10-dec-none-gcc
STDBUF ?= stdbuf
WATCH_OUTPUT := $(PROJECT_ROOT)/tests/simh/watch-output.sh

KINIT_NAMES = kinit kinit_diag module_minit kinit_io module_io
KINIT_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_NAMES)))
KINIT_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_NAMES)))
KCORE_FIXED_NAMES = kcore_entry kcore kcore_pi kcore_io kcore_end
KCORE_FIXED_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(KCORE_FIXED_NAMES)))
KCORE_FIXED_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KCORE_FIXED_NAMES)))
MRES_NAMES = cty clk ptr ptp cr cp cty_io clk_io ptr_io ptp_io cr_io cp_io wcnsls_io
MRES_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(MRES_NAMES)))

KINIT_INPUTS = $(addprefix $(PROJECT_ROOT)/system/kernel/, \
	kinit.c kinit_diag.c module_minit.c kinit.h module.h mres.h \
	cty.h clk.h pt.h card.h wcnsls.h kinit_io.s module_io.s)
KCORE_INPUTS = $(addprefix $(PROJECT_ROOT)/system/kernel/, \
	kcore.c kcore_pi.c cty.c clk.c ptr.c ptp.c cr.c cp.c \
	kcore.h kcore_pi.h kcore_io.h cty.h clk.h pt.h card.h wcnsls.h \
	kcore_entry.s kcore_io.s kcore_end.s cty_io.s clk_io.s ptr_io.s \
	ptp_io.s cr_io.s cp_io.s wcnsls_io.s)

KCORE_DXR = $(BUILD)/kcore-v1.dxr
KCORE_MAP = $(BUILD)/kcore-v1.map
KCORE_RIM = $(BUILD)/kcore-v1.rim
KCORE_STREAM = $(BUILD)/kcore-v1.stream
KCORE_EMBED_ASM = $(BUILD)/kcore-embed-v1.s
KCORE_EMBED_OBJ = $(BUILD)/kcore-embed-v1.dobj

CTY_MRES_OBJ = $(BUILD)/cty-mres.dobj
CLK_MRES_OBJ = $(BUILD)/clk-mres.dobj
PTR_MRES_OBJ = $(BUILD)/ptr-mres.dobj
PTP_MRES_OBJ = $(BUILD)/ptp-mres.dobj
CR_MRES_OBJ = $(BUILD)/cr-mres.dobj
CP_MRES_OBJ = $(BUILD)/cp-mres.dobj
WCNSLS_MRES_OBJ = $(BUILD)/wcnsls-mres.dobj
MRES_PACKAGE_OBJS = $(CTY_MRES_OBJ) $(CLK_MRES_OBJ) $(PTR_MRES_OBJ) \
	$(PTP_MRES_OBJ) $(CR_MRES_OBJ) $(CP_MRES_OBJ) $(WCNSLS_MRES_OBJ)

MINIT_TABLE_OBJ = $(BUILD)/module-table-test.dobj
DEVICE_TEST_ASM = $(KINIT_BUILD)/device-test-minit.s
DEVICE_TEST_OBJ = $(BUILD)/device-test-minit.dobj
END_OBJ = $(BUILD)/image-end-v1.dobj
IMAGE_DXR = $(BUILD)/kinit-v1.dxr
IMAGE_MAP = $(BUILD)/kinit-v1.map
IMAGE_RIM = $(BUILD)/kinit-v1.rim
IMAGE_RAW = $(BUILD)/kinit-v1.raw.words
IMAGE_WORDS = $(BUILD)/kinit-v1.words
STAGE1_DXR = $(BUILD)/stage1-dsk-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dsk-v1.pt
DISK_DIR = $(BUILD)/disk
DISK_IMAGE = $(DISK_DIR)/dsk0.dsk
SIMH_INI = $(BUILD)/kinit-v1.ini
SIMH_INI_NO_PTP = $(BUILD)/kinit-v1-no-ptp.ini
SIMH_INI_NO_SLV = $(BUILD)/kinit-v1-no-slv.ini
LOG = $(BUILD)/kinit-v1.log
LOG_NO_PTP = $(BUILD)/kinit-v1-no-ptp.log
LOG_NO_SLV = $(BUILD)/kinit-v1-no-slv.log
SIMH_LOG = $(BUILD)/kinit-v1.simh.log
SIMH_LOG_NO_PTP = $(BUILD)/kinit-v1-no-ptp.simh.log
SIMH_LOG_NO_SLV = $(BUILD)/kinit-v1-no-slv.simh.log
PTP_OUTPUT = $(BUILD)/ptp-output.bin
CR_INPUT = $(BUILD)/cr-input.txt
CP_OUTPUT = $(BUILD)/cp-output.txt

.DEFAULT_GOAL := image
.PHONY: image test clean

$(BUILD):
	mkdir -p $@

$(KINIT_ASM): $(KINIT_INPUTS) | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kinit BUILD='$(KINIT_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)' KINIT_DEBUG='$(KINIT_DEBUG)'

$(KCORE_FIXED_ASM) $(MRES_ASM): $(KCORE_INPUTS) | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kcore BUILD='$(KCORE_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)'

$(BUILD)/%.dobj: $(KINIT_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/%.dobj: $(KCORE_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(DEVICE_TEST_ASM): device-test-minit.c $(PROJECT_ROOT)/system/kernel/kinit.h \
    $(PROJECT_ROOT)/system/kernel/module.h $(PROJECT_ROOT)/system/kernel/card.h \
    $(PROJECT_ROOT)/system/kernel/wcnsls.h | $(BUILD)
	mkdir -p $(KINIT_BUILD)
	$(PDP10_GCC) -std=c99 -Os -I$(PROJECT_ROOT)/system/kernel -I$(PDP10_PREFIX)/include -S $< -o $@
$(DEVICE_TEST_OBJ): $(DEVICE_TEST_ASM) | $(BUILD)
	$(DAS) -c -o $@ $<
$(MINIT_TABLE_OBJ): module-table-test.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(END_OBJ): image-end-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(KCORE_DXR): $(KCORE_FIXED_OBJS)
	$(DLINK) -b 060 -o $@ -M $(KCORE_MAP) $(KCORE_FIXED_OBJS)
$(KCORE_MAP): $(KCORE_DXR)
	@test -f $@
$(KCORE_RIM): $(KCORE_DXR)
	$(DXRCONVERT) --simh -b 060 $< $@
$(KCORE_STREAM): $(KCORE_RIM) $(KCORE_MAP)
	end=`awk '$$1 == "__kcore_low_init_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o60)[2:])' $$end`; \
	$(MKSTREAM) -i $(KCORE_RIM) -o $@ -b 060 -w "$$words"

$(CTY_MRES_OBJ): $(BUILD)/cty.dobj $(BUILD)/cty_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cty-mres.dxr -M $(BUILD)/cty-mres.map \
	    -R $@ -N cty_mres_package -X cty_pi_handler -X cty_put6 \
	    -X cty_newline -X cty_put6_spaces $(BUILD)/cty.dobj $(BUILD)/cty_io.dobj
$(CLK_MRES_OBJ): $(BUILD)/clk.dobj $(BUILD)/clk_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/clk-mres.dxr -M $(BUILD)/clk-mres.map \
	    -R $@ -N clk_mres_package -X clk_pi_handler -X clk_ticks $(BUILD)/clk.dobj $(BUILD)/clk_io.dobj
$(PTR_MRES_OBJ): $(BUILD)/ptr.dobj $(BUILD)/ptr_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ptr-mres.dxr -M $(BUILD)/ptr-mres.map \
	    -R $@ -N ptr_mres_package -X ptr_pi_handler -X ptr_getchar $(BUILD)/ptr.dobj $(BUILD)/ptr_io.dobj
$(PTP_MRES_OBJ): $(BUILD)/ptp.dobj $(BUILD)/ptp_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/ptp-mres.dxr -M $(BUILD)/ptp-mres.map \
	    -R $@ -N ptp_mres_package -X ptp_pi_handler -X ptp_putchar $(BUILD)/ptp.dobj $(BUILD)/ptp_io.dobj
$(CR_MRES_OBJ): $(BUILD)/cr.dobj $(BUILD)/cr_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cr-mres.dxr -M $(BUILD)/cr-mres.map \
	    -R $@ -N cr_mres_package -X cr_pi_handler -X cr_read_card $(BUILD)/cr.dobj $(BUILD)/cr_io.dobj
$(CP_MRES_OBJ): $(BUILD)/cp.dobj $(BUILD)/cp_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/cp-mres.dxr -M $(BUILD)/cp-mres.map \
	    -R $@ -N cp_mres_package -X cp_pi_handler -X cp_punch_card $(BUILD)/cp.dobj $(BUILD)/cp_io.dobj
$(WCNSLS_MRES_OBJ): $(BUILD)/wcnsls_io.dobj $(KCORE_MAP)
	$(DLINK) -b 0 -A $(KCORE_MAP) -o $(BUILD)/wcnsls-mres.dxr -M $(BUILD)/wcnsls-mres.map \
	    -R $@ -N wcnsls_mres_package -X wcnsls_read -X wcnsls_cono -X wcnsls_plot $(BUILD)/wcnsls_io.dobj

$(KCORE_EMBED_ASM): kcore-embed-v1.s.in $(KCORE_MAP) | $(BUILD)
	end=`awk '$$1 == "__kcore_low_init_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o60)[2:])' $$end`; \
	sed "s/@KCORE_INIT_WORDS@/$$words/" $< > $@
$(KCORE_EMBED_OBJ): $(KCORE_EMBED_ASM)
	$(DAS) -c -o $@ $<

$(IMAGE_DXR): $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
    $(MRES_PACKAGE_OBJS) $(DEVICE_TEST_OBJ) $(END_OBJ) $(KCORE_MAP)
	$(DLINK) -b 040000 -A $(KCORE_MAP) -o $@ -M $(IMAGE_MAP) \
	    $(MINIT_TABLE_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) \
	    $(MRES_PACKAGE_OBJS) $(DEVICE_TEST_OBJ) $(END_OBJ)
$(IMAGE_MAP): $(IMAGE_DXR)
	@test -f $@
$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 040000 $< $@
$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	end=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o40000)[2:])' $$end`; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 040000 -w "$$words"
$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(KCORE_STREAM) $(KCORE_MAP) mkimage-v1.py
	$(PYTHON) mkimage-v1.py --map $(IMAGE_MAP) --input $(IMAGE_RAW) \
	    --kcore $(KCORE_STREAM) --kcore-map $(KCORE_MAP) --output $@

STAGE1_SRC := $(PROJECT_ROOT)/system/stand/pdp6/dsk/stage1.s
$(STAGE1_DXR): $(STAGE1_SRC) | $(BUILD)
	$(DAS) --base-kernel -o $@ $<
$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b 060 $< $@
$(DISK_IMAGE): $(IMAGE_WORDS) $(STAGE1_TAPE)
	rm -rf $(DISK_DIR)
	$(MKDSK) -n 1 -m clean -p $(IMAGE_WORDS) -o $(DISK_DIR)
$(CR_INPUT): | $(BUILD)
	printf 'DAIMOS CARD TEST\n' > $@

$(SIMH_INI): simh-v1.ini.in $(STAGE1_TAPE) $(DISK_IMAGE) $(CR_INPUT)
	sed -e 's|@SIMH_LOG@|$(abspath $(SIMH_LOG))|g' \
	    -e 's|@STAGE1_TAPE@|$(abspath $(STAGE1_TAPE))|g' \
	    -e 's|@DISK_IMAGE@|$(abspath $(DISK_IMAGE))|g' \
	    -e 's|@PTP_OUTPUT@|$(abspath $(PTP_OUTPUT))|g' \
	    -e 's|@CR_INPUT@|$(abspath $(CR_INPUT))|g' \
	    -e 's|@CP_OUTPUT@|$(abspath $(CP_OUTPUT))|g' $< > $@
$(SIMH_INI_NO_PTP): $(SIMH_INI)
	sed -e 's/set ptp enabled/set ptp disabled/' -e '/attach -q -n ptp/d' $< > $@
$(SIMH_INI_NO_SLV): $(SIMH_INI)
	sed 's/set slave enabled/set slave disabled/' $< > $@

image: $(SIMH_INI)

test: image $(SIMH_INI_NO_PTP) $(SIMH_INI_NO_SLV)
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI) $(SIMH_LOG) $(LOG) \
	    'DAIMON                              V0.1' \
	    'MACH                                PDP6' \
	    'MEM                                256 K' \
	    'CTY                                   OK' \
	    'HZ                               60 LINE' \
	    'PTR                                   OK' \
	    'PTP                                   OK' \
	    'CR                                    OK' \
	    'CP                                    OK' \
	    'WCNSLS' \
	    'SLV                               NO DRV' \
	    'HALT instruction' $(if $(filter 1,$(KINIT_DEBUG)),>/dev/null,)
ifeq ($(KINIT_DEBUG),1)
	@cat $(LOG)
endif
	@$(PYTHON) -c "from pathlib import Path; p=Path('$(PTP_OUTPUT)').read_bytes(); assert p == bytes([0o252]), p"
	@test -s $(CP_OUTPUT)
	@grep 'DAIMOS CARD TEST' $(CP_OUTPUT) >/dev/null
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_NO_PTP) $(SIMH_LOG_NO_PTP) $(LOG_NO_PTP) \
	    'PTP                               NO DEV' \
	    'HALT instruction' >/dev/null
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI_NO_SLV) $(SIMH_LOG_NO_SLV) $(LOG_NO_SLV) \
	    'SLV                               NO DEV' \
	    'HALT instruction' >/dev/null
	@echo 'KINIT conditional-MRES device checkpoint test PASS'

clean:
	rm -rf $(BUILD)
