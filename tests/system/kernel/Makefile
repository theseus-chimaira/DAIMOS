PROJECT_ROOT := $(abspath ../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/kernel
KINIT_BUILD := $(BUILD)/kinit-asm
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

KINIT_NAMES = kinit kinit_boot kinit_diag kinit_reloc kinit_io
KINIT_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_NAMES)))
KINIT_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_NAMES)))
KCORE_NAMES = kcore_entry kcore kcore_pi kcore_cty kcore_io kcore_end
KCORE_ASM = $(addprefix $(KCORE_BUILD)/,$(addsuffix .s,$(KCORE_NAMES)))
KCORE_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KCORE_NAMES)))
KCORE_DXR = $(BUILD)/kcore-v1.dxr
KCORE_MAP = $(BUILD)/kcore-v1.map
KCORE_RIM = $(BUILD)/kcore-v1.rim
KCORE_STREAM = $(BUILD)/kcore-v1.stream
KCORE_EMBED_ASM = $(BUILD)/kcore-embed-v1.s
KCORE_EMBED_OBJ = $(BUILD)/kcore-embed-v1.dobj
PREFIX_OBJ = $(BUILD)/image-v1.dobj
MINIT_ASM = $(BUILD)/minit-v1.s
MINIT_OBJ = $(BUILD)/minit-v1.dobj
END_OBJ = $(BUILD)/image-end-v1.dobj
IMAGE_DXR = $(BUILD)/kinit-pre-reloc-v1.dxr
IMAGE_MAP = $(BUILD)/kinit-pre-reloc-v1.map
IMAGE_RIM = $(BUILD)/kinit-pre-reloc-v1.rim
IMAGE_RAW = $(BUILD)/kinit-pre-reloc-v1.raw.words
IMAGE_WORDS = $(BUILD)/kinit-pre-reloc-v1.words
STAGE1_DXR = $(BUILD)/stage1-dsk-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dsk-v1.pt
DISK_DIR = $(BUILD)/disk
DISK_IMAGE = $(DISK_DIR)/dsk0.dsk
SIMH_INI = $(BUILD)/kinit-pre-reloc-v1.ini
LOG = $(BUILD)/kinit-pre-reloc-v1.log
SIMH_LOG = $(BUILD)/kinit-pre-reloc-v1.simh.log

.DEFAULT_GOAL := image
.PHONY: image test clean

$(BUILD):
	mkdir -p $@

$(KINIT_ASM): | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kinit BUILD='$(KINIT_BUILD)'

$(KCORE_ASM): | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kcore BUILD='$(KCORE_BUILD)'

$(BUILD)/kinit.dobj: $(KINIT_BUILD)/kinit.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kinit_boot.dobj: $(KINIT_BUILD)/kinit_boot.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kinit_diag.dobj: $(KINIT_BUILD)/kinit_diag.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kinit_reloc.dobj: $(KINIT_BUILD)/kinit_reloc.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kinit_io.dobj: $(KINIT_BUILD)/kinit_io.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(BUILD)/kcore_entry.dobj: $(KCORE_BUILD)/kcore_entry.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kcore.dobj: $(KCORE_BUILD)/kcore.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kcore_pi.dobj: $(KCORE_BUILD)/kcore_pi.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kcore_cty.dobj: $(KCORE_BUILD)/kcore_cty.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kcore_io.dobj: $(KCORE_BUILD)/kcore_io.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kcore_end.dobj: $(KCORE_BUILD)/kcore_end.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(KCORE_DXR): $(KCORE_OBJS)
	$(DLINK) -o $@ -M $(KCORE_MAP) $(KCORE_OBJS)

$(KCORE_RIM): $(KCORE_DXR)
	$(DXRCONVERT) --simh -b 060 $< $@

$(KCORE_STREAM): $(KCORE_RIM) $(KCORE_MAP)
	words=`awk '$$1 == "__kcore_image_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$words"; \
	$(MKSTREAM) -i $(KCORE_RIM) -o $@ -b 060 -w "$$words"

$(KCORE_EMBED_ASM): $(KCORE_STREAM) $(KCORE_MAP)
	words=`awk '$$1 == "__kcore_image_end" { print $$2 }' $(KCORE_MAP)`; \
	test -n "$$words"; \
	{ \
		echo '.text'; \
		echo '.globl test_kcore'; \
		echo '.globl test_kcore_end'; \
		echo '.globl __kinit_code_start'; \
		echo 'test_kcore:'; \
		echo "        .block $$words"; \
		echo 'test_kcore_end:'; \
		echo '__kinit_code_start:'; \
	} > $@

$(KCORE_EMBED_OBJ): $(KCORE_EMBED_ASM)
	$(DAS) -c -o $@ $<

$(PREFIX_OBJ): image-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(END_OBJ): image-end-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(MINIT_ASM): minit-v1.c | $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -S $< -o $@

$(MINIT_OBJ): $(MINIT_ASM)
	$(DAS) -c -o $@ $<

$(IMAGE_DXR): $(PREFIX_OBJ) $(MINIT_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) $(END_OBJ)
	$(DLINK) -o $@ -M $(IMAGE_MAP) $(PREFIX_OBJ) $(MINIT_OBJ) $(KCORE_EMBED_OBJ) $(KINIT_OBJS) $(END_OBJ)

$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 040000 $< $@

$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	words=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$words"; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 040000 -w "$$words"

$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(KCORE_STREAM) mkimage-v1.py
	$(PYTHON) mkimage-v1.py --map $(IMAGE_MAP) --input $(IMAGE_RAW) --kcore $(KCORE_STREAM) --output $@

STAGE1_SRC := $(PROJECT_ROOT)/system/stand/pdp6/dsk/stage1.s

$(STAGE1_DXR): $(STAGE1_SRC) | $(BUILD)
	$(DAS) --base-kernel -o $@ $<

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b 060 $< $@

$(DISK_IMAGE): $(IMAGE_WORDS) $(STAGE1_TAPE)
	rm -rf $(DISK_DIR)
	$(MKDSK) -n 1 -m clean -p $(IMAGE_WORDS) -o $(DISK_DIR)

$(SIMH_INI): simh-v1.ini.in $(STAGE1_TAPE) $(DISK_IMAGE)
	sed -e 's|@SIMH_LOG@|$(abspath $(SIMH_LOG))|g' \
	    -e 's|@STAGE1_TAPE@|$(abspath $(STAGE1_TAPE))|g' \
	    -e 's|@DISK_IMAGE@|$(abspath $(DISK_IMAGE))|g' \
	    $< > $@

image: $(SIMH_INI)

test: image
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 5 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI) $(SIMH_LOG) $(LOG) \
	    'DAIMON V0.1' \
	    'KCORE' \
	    'KINIT' \
	    'MACH' \
	    '256 K CORE' \
	    'CONS' \
	    'CTY0' \
	    'MINIT TEST' \
	    'KINIT FINISHED' \
	    'HALT instruction'
	@echo 'KINIT MINIT checkpoint test PASS'

clean:
	rm -rf $(BUILD)
