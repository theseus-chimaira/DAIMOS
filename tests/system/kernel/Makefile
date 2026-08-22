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

KINIT_NAMES = kinit kinit_diag kinit_io
KINIT_ASM = $(addprefix $(KINIT_BUILD)/,$(addsuffix .s,$(KINIT_NAMES)))
KINIT_OBJS = $(addprefix $(BUILD)/,$(addsuffix .dobj,$(KINIT_NAMES)))
KCORE_ASM = $(KCORE_BUILD)/kcore_entry.s $(KCORE_BUILD)/kcore.s $(KCORE_BUILD)/resident_end.s
KCORE_OBJS = $(BUILD)/kcore_entry.dobj $(BUILD)/kcore.dobj $(BUILD)/resident-v1.dobj $(BUILD)/resident_end.dobj
RESIDENT_DXR = $(BUILD)/resident-v1.dxr
RESIDENT_MAP = $(BUILD)/resident-v1.map
RESIDENT_RIM = $(BUILD)/resident-v1.rim
RESIDENT_STREAM = $(BUILD)/resident-v1.stream
RESIDENT_EMBED_ASM = $(BUILD)/resident-embed-v1.s
RESIDENT_EMBED_OBJ = $(BUILD)/resident-embed-v1.dobj
MINIT_ASM = $(BUILD)/minit-v1.s
MINIT_OBJ = $(BUILD)/minit-v1.dobj
MINIT_TABLE_OBJ = $(BUILD)/minit-table-v1.dobj
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
LOG = $(BUILD)/kinit-v1.log
SIMH_LOG = $(BUILD)/kinit-v1.simh.log

.DEFAULT_GOAL := image
.PHONY: image test clean

$(BUILD):
	mkdir -p $@

$(KINIT_ASM): | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kinit BUILD='$(KINIT_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)'

$(KCORE_ASM): | $(BUILD)
	$(MAKE) -C $(PROJECT_ROOT)/system/kernel kcore BUILD='$(KCORE_BUILD)' PDP10_PREFIX='$(PDP10_PREFIX)'

$(BUILD)/%.dobj: $(KINIT_BUILD)/%.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(BUILD)/kcore_entry.dobj: $(KCORE_BUILD)/kcore_entry.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/kcore.dobj: $(KCORE_BUILD)/kcore.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(BUILD)/resident_end.dobj: $(KCORE_BUILD)/resident_end.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(BUILD)/resident-v1.s: resident-v1.c | $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -S $< -o $@
$(BUILD)/resident-v1.dobj: $(BUILD)/resident-v1.s
	$(DAS) -c -o $@ $<

$(RESIDENT_DXR): $(KCORE_OBJS)
	$(DLINK) -b 060 -o $@ -M $(RESIDENT_MAP) $(KCORE_OBJS)

$(RESIDENT_MAP): $(RESIDENT_DXR)
	@test -f $@

$(RESIDENT_RIM): $(RESIDENT_DXR)
	$(DXRCONVERT) --simh -b 060 $< $@

$(RESIDENT_STREAM): $(RESIDENT_RIM) $(RESIDENT_MAP)
	end=`awk '$$1 == "__resident_low_init_end" { print $$2 }' $(RESIDENT_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o60)[2:])' $$end`; test -n "$$words"; \
	$(MKSTREAM) -i $(RESIDENT_RIM) -o $@ -b 060 -w "$$words"

$(RESIDENT_EMBED_ASM): resident-embed-v1.s.in $(RESIDENT_MAP) | $(BUILD)
	end=`awk '$$1 == "__resident_low_init_end" { print $$2 }' $(RESIDENT_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o60)[2:])' $$end`; test -n "$$words"; \
	sed "s/@RESIDENT_INIT_WORDS@/$$words/" $< > $@

$(RESIDENT_EMBED_OBJ): $(RESIDENT_EMBED_ASM)
	$(DAS) -c -o $@ $<

$(MINIT_TABLE_OBJ): minit-table-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<
$(END_OBJ): image-end-v1.s | $(BUILD)
	$(DAS) -c -o $@ $<

$(MINIT_ASM): minit-v1.c | $(BUILD)
	$(PDP10_GCC) -std=c99 -Os -I$(PROJECT_ROOT)/system/kernel -I$(PDP10_PREFIX)/include -S $< -o $@
$(MINIT_OBJ): $(MINIT_ASM)
	$(DAS) -c -o $@ $<

$(IMAGE_DXR): $(MINIT_TABLE_OBJ) $(MINIT_OBJ) $(MINIT_TABLE_OBJ) $(RESIDENT_EMBED_OBJ) $(KINIT_OBJS) $(END_OBJ) $(RESIDENT_MAP)
	$(DLINK) -b 040000 -A $(RESIDENT_MAP) -o $@ -M $(IMAGE_MAP) \
	    $(MINIT_TABLE_OBJ) $(MINIT_OBJ) $(RESIDENT_EMBED_OBJ) $(KINIT_OBJS) $(END_OBJ)

$(IMAGE_MAP): $(IMAGE_DXR)
	@test -f $@

$(IMAGE_RIM): $(IMAGE_DXR)
	$(DXRCONVERT) --simh -b 040000 $< $@

$(IMAGE_RAW): $(IMAGE_RIM) $(IMAGE_MAP)
	end=`awk '$$1 == "__kinit_image_end" { print $$2 }' $(IMAGE_MAP)`; \
	test -n "$$end"; words=`$(PYTHON) -c 'import sys; print(oct(int(sys.argv[1],8)-0o40000)[2:])' $$end`; test -n "$$words"; \
	$(MKSTREAM) -i $(IMAGE_RIM) -o $@ -b 040000 -w "$$words"

$(IMAGE_WORDS): $(IMAGE_RAW) $(IMAGE_MAP) $(RESIDENT_STREAM) $(RESIDENT_MAP) mkimage-v1.py
	$(PYTHON) mkimage-v1.py --map $(IMAGE_MAP) --input $(IMAGE_RAW) \
	    --resident $(RESIDENT_STREAM) --resident-map $(RESIDENT_MAP) --output $@

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
	    -e 's|@DISK_IMAGE@|$(abspath $(DISK_IMAGE))|g' $< > $@

image: $(SIMH_INI)

test: image
	@STDBUF='$(STDBUF)' $(WATCH_OUTPUT) -f 6 -t 2 \
	    $(SIMH_PDP6) $(SIMH_INI) $(SIMH_LOG) $(LOG) \
	    'DAIMON                              V0.1' \
	    'MACH                                PDP6' \
	    'MEM                                256 K' \
	    'MINIT1' \
	    'MINIT2' \
	    'HALT instruction'
	@echo 'KINIT linked-resident/MINIT checkpoint test PASS'

clean:
	rm -rf $(BUILD)
