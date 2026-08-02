include ../bin/stat.mk

STAGE1 = ../../ptr/stage1.s
STAGE1_ENTRY = 060000
STAGE1_RIM = $(BUILD)/stage1-ptr.rim
STAGE1_TAPE = $(BUILD)/stage1-ptr-v1.pt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in

.PHONY: all image test clean

all: image

$(STAGE1_RIM): $(STAGE1) | $(BUILD)
	$(PDP10_AS) --start start --at $(STAGE1_ENTRY) $(STAGE1) > $@

$(BUILD)/mkpdp6rim-v1: $(TOOLS_DIR)/mkpdp6rim_v1.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/mkpdp6rim_v1.c

$(STAGE1_TAPE): $(STAGE1_RIM) $(BUILD)/mkpdp6rim-v1
	$(BUILD)/mkpdp6rim-v1 $(STAGE1_RIM) $@

$(SIMH_INI): $(SIMH_TEMPLATE) $(STAGE1_TAPE) | $(BUILD)
	sed -e 's|@SIMH_LOG@|$(BUILD)/ptr-v1.simh.log|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@STAGE1_ENTRY@|$(STAGE1_ENTRY)|g' \
	    $(SIMH_TEMPLATE) > $@

image: $(STAGE1_TAPE) $(SIMH_INI)

test: image
	@log="$(BUILD)/ptr-v1.log"; \
	timeout $(TIMEOUT) $(SIMH_PDP6) $(SIMH_INI) > "$$log" 2>&1 || \
	    { cat "$$log"; exit 1; }; \
	grep -F "Breakpoint, PC: 060000" "$$log" >/dev/null || \
	    { cat "$$log"; exit 1; }

clean:
	rm -rf $(BUILD)
