PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/dtc

include ../bin/stat.mk

STAGE1 = ../../dtc/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-dtc-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dtc-v1.pt
PAYLOAD_TAPE = $(BUILD)/payload-v1.dt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in

.DEFAULT_GOAL := build

.PHONY: build image test clean

build: image

$(STAGE1_DXR): $(STAGE1) | $(BUILD)
	$(DAS) --base-kernel -o $@ $(STAGE1)

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b $(STAGE1_ENTRY) $(STAGE1_DXR) $@

$(PAYLOAD_TAPE): $(STAT_WORDS) $(MKDT)
	$(MKDT) -t dtc -p $(STAT_WORDS) -o $@

$(SIMH_INI): $(SIMH_TEMPLATE) $(STAGE1_TAPE) $(PAYLOAD_TAPE) | $(BUILD)
	sed -e 's|@SIMH_LOG@|$(BUILD)/dtc-v1.simh.log|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@PAYLOAD_TAPE@|$(PAYLOAD_TAPE)|g' \
	    $(SIMH_TEMPLATE) > $@

image: $(STAGE1_TAPE) $(PAYLOAD_TAPE) $(SIMH_INI)

test: image
	@log="$(BUILD)/dtc-v1.log"; \
	simhlog="$(BUILD)/dtc-v1.simh.log"; \
	echo "DTC dtc-v1"; \
	start=$$(date +%s); \
	timeout $(TIMEOUT) $(SIMH_PDP6) $(SIMH_INI) < /dev/null > "$$log" 2>&1; \
	rc=$$?; \
	reason=; \
	if test "$$rc" -ne 0; then reason="SIMH exit $$rc"; fi; \
	if test -z "$$reason" && ! grep -F "BOOTLOADER OK" "$$log" >/dev/null; then \
	    reason="missing BOOTLOADER OK"; \
	fi; \
	end=$$(date +%s); elapsed=$$((end - start)); \
	if test -n "$$reason"; then \
	    test -z "$$TEST_REPORT_DETAIL" || \
	      printf '%-6s %-4s %-18s %7ss  %s; log=%s\n' FAIL DTC dtc-v1 "$$elapsed" "$$reason" "$$log" >> "$$TEST_REPORT_DETAIL"; \
	    echo "DTC dtc-v1 failed: $$reason"; \
	    cat "$$log"; \
	    cat "$$simhlog"; \
	    exit 1; \
	fi; \
	test -z "$$TEST_REPORT_DETAIL" || \
	  printf '%-6s %-4s %-18s %7ss  bootloader and payload verified\n' PASS DTC dtc-v1 "$$elapsed" >> "$$TEST_REPORT_DETAIL"

clean:
	rm -rf $(BUILD)
