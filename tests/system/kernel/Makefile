PROJECT_ROOT := $(abspath ../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/kernel
KINIT_BUILD := $(BUILD)/kinit-asm
PDP10_PREFIX ?= $(HOME)/cross

DAS ?= $(PDP10_PREFIX)/bin/das
DLINK ?= $(PDP10_PREFIX)/bin/dlink
DXRCONVERT ?= $(PDP10_PREFIX)/bin/dxrconvert
MKSTREAM ?= $(PDP10_PREFIX)/bin/mkstream
MKDSK ?= $(PDP10_PREFIX)/bin/mkdsk
SIMH_PDP6 ?= $(PDP10_PREFIX)/bin/pdp6
PYTHON ?= python3
STDBUF ?= stdbuf
WATCH_OUTPUT := $(PROJECT_ROOT)/tests/simh/watch-output.sh

KINIT_NAMES = kinit kinit_boot kinit_diag kinit_reloc kinit_io
KINIT_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_NAMES)))
KINIT_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_NAMES)))
PREFIX_OBJ = $(BUILD)/image-v1.dobj
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

$(BUILD)/%.dobj: $(KINIT_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(PREFIX_OBJ): image-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(END_OBJ): image-end-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(IMAGE_DXR): $(PREFIX_OBJ) $(KINIT_OBJS) $(END_OBJ)
	$(DLINK) -o $@ -M $(IMAGE_MAP) $(PREFIX_OBJ) $(KINIT_OBJS) $(END_OBJ)

$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 040000 $< $@

$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	words=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$words"; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 040000 -w "$$words"

$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) mkimage-v1.py
	$(PYTHON) mkimage-v1.py --map $(IMAGE_MAP) --input $(IMAGE_RAW) --output $@

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
	    'CTY' \
	    'HALT instruction'
	@if grep -F 'KINIT FINISHED' $(SIMH_LOG) >/dev/null; then \
	    echo 'relocation checkpoint was passed unexpectedly'; \
	    cat $(SIMH_LOG); \
	    exit 1; \
	fi
	@echo 'KINIT pre-relocation test PASS'

clean:
	rm -rf $(BUILD)
