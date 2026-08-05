include ../bin/stat.mk

STAGE1 = ../../ptr/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-ptr-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-ptr-v1.pt
PAYLOAD_TAPE = $(BUILD)/payload-v1.pt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in

.PHONY: all image test clean

all: image

$(STAGE1_DXR): $(STAGE1) | $(BUILD)
	$(DAS) --base-kernel -o $@ $(STAGE1)

$(BUILD)/dxr2pdp6rim-v1: $(TOOLS_DIR)/dxr2pdp6rim_v1.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/dxr2pdp6rim_v1.c

$(STAGE1_TAPE): $(STAGE1_DXR) $(BUILD)/dxr2pdp6rim-v1
	$(BUILD)/dxr2pdp6rim-v1 -b $(STAGE1_ENTRY) $(STAGE1_DXR) $@

$(PAYLOAD_TAPE): $(STAT_WORDS) $(BUILD)/words2pt
	$(BUILD)/words2pt -t ptr -p $(STAT_WORDS) -o $@

$(SIMH_INI): $(SIMH_TEMPLATE) $(STAGE1_TAPE) $(PAYLOAD_TAPE) | $(BUILD)
	sed -e 's|@SIMH_LOG@|$(BUILD)/ptr-v1.simh.log|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@PAYLOAD_TAPE@|$(PAYLOAD_TAPE)|g' \
	    -e 's|@STAGE1_ENTRY@|$(STAGE1_ENTRY)|g' \
	    $(SIMH_TEMPLATE) > $@

image: $(STAGE1_TAPE) $(PAYLOAD_TAPE) $(SIMH_INI)

test: image
	@log="$(BUILD)/ptr-v1.log"; \
	simhlog="$(BUILD)/ptr-v1.simh.log"; \
	echo "PTR ptr-v1"; \
	timeout $(TIMEOUT) $(SIMH_PDP6) $(SIMH_INI) < /dev/null > "$$log" 2>&1; \
	rc=$$?; \
	if test "$$rc" -ne 0 && test "$$rc" -ne 124; then \
	    echo "SIMH failed for ptr-v1: exit $$rc"; \
	    echo "command: timeout $(TIMEOUT) $(SIMH_PDP6) $(SIMH_INI) < /dev/null"; \
	    cat "$$log"; \
	    cat "$$simhlog"; \
	    exit 1; \
	fi; \
	grep -F "BOOTLOADER OK" "$$log" >/dev/null || \
	    { cat "$$log"; cat "$$simhlog"; exit 1; }

clean:
	rm -rf $(BUILD)
