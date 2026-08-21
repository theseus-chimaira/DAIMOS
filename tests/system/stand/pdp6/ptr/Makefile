PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/stand/pdp6/ptr

include ../bin/stat.mk

BOOT_KIND = PTR
TEST_NAME = ptr-two-tape-v1
STAGE1 = $(PROJECT_ROOT)/system/stand/pdp6/ptr/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-ptr-two-tape-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-ptr-two-tape-v1.pt

PAYLOAD1_SRC = payload1-v1.s
PAYLOAD1_DXR = $(BUILD)/payload1-v1.dxr
PAYLOAD1_RIM = $(BUILD)/payload1-v1.rim
PAYLOAD1_STREAM = $(BUILD)/payload1-v1.stream
PAYLOAD1_WORDS = $(BUILD)/payload1-v1.words

PAYLOAD2_SRC = payload2-v1.s
PAYLOAD2_DXR = $(BUILD)/payload2-v1.dxr
PAYLOAD2_RIM = $(BUILD)/payload2-v1.rim
PAYLOAD2_STREAM = $(BUILD)/payload2-v1.stream
PAYLOAD2_WORDS = $(BUILD)/payload2-v1.words

TAPE1_WORDS = $(BUILD)/tape1-v1.words
TAPE2_WORDS = $(BUILD)/tape2-v1.words
TAPE1 = $(BUILD)/tape1-v1.pt
TAPE2 = $(BUILD)/tape2-v1.pt
SIMH_INI = $(BUILD)/simh-v1.ini
SIMH_TEMPLATE = simh-v1.ini.in

.DEFAULT_GOAL := build

.PHONY: build image test clean

$(STAGE1_DXR): $(STAGE1) | $(BUILD)
	$(DAS) --base-kernel -o $@ $<

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b $(STAGE1_ENTRY) $< $@

$(PAYLOAD1_DXR): $(PAYLOAD1_SRC) | $(BUILD)
	$(DAS) --base-kernel -o $@ $<

$(PAYLOAD1_RIM): $(PAYLOAD1_DXR)
	$(DXRCONVERT) --simh -b 040000 $< $@

$(PAYLOAD1_STREAM): $(PAYLOAD1_RIM)
	$(MKSTREAM) -i $< -o $@ -b 040000 -w 014

$(PAYLOAD1_WORDS): $(PAYLOAD1_STREAM)
	tail -n +3 $< > $@
	test `wc -l < $@` -eq 12

$(PAYLOAD2_DXR): $(PAYLOAD2_SRC) | $(BUILD)
	$(DAS) --base-kernel -o $@ $<

$(PAYLOAD2_RIM): $(PAYLOAD2_DXR)
	$(DXRCONVERT) --simh -b 040014 $< $@

$(PAYLOAD2_STREAM): $(PAYLOAD2_RIM)
	$(MKSTREAM) -i $< -o $@ -b 040014 -w 017

$(PAYLOAD2_WORDS): $(PAYLOAD2_STREAM)
	tail -n +3 $< > $@
	test `wc -l < $@` -eq 15

$(TAPE1_WORDS): $(PAYLOAD1_WORDS) | $(BUILD)
	{ printf '%s\n' 000000000014; cat $(PAYLOAD1_WORDS); } > $@
	test `wc -l < $@` -eq 13

$(TAPE2_WORDS): $(PAYLOAD2_WORDS) | $(BUILD)
	{ printf '%s\n' 000000000017; cat $(PAYLOAD2_WORDS); } > $@
	test `wc -l < $@` -eq 16

$(TAPE1): $(TAPE1_WORDS) $(WORDS2PT)
	$(WORDS2PT) -t ptr -p $< -o $@

$(TAPE2): $(TAPE2_WORDS) $(WORDS2PT)
	$(WORDS2PT) -t ptr -p $< -o $@

$(SIMH_INI): $(SIMH_TEMPLATE) $(STAGE1_TAPE) $(TAPE1) $(TAPE2) | $(BUILD)
	sed -e 's|@SIMH_LOG@|$(BUILD)/$(TEST_NAME).simh.log|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@TAPE1@|$(TAPE1)|g' \
	    -e 's|@TAPE2@|$(TAPE2)|g' \
	    -e 's|@STAGE1_ENTRY@|$(STAGE1_ENTRY)|g' \
	    $(SIMH_TEMPLATE) > $@

image: $(STAGE1_TAPE) $(TAPE1) $(TAPE2) $(SIMH_INI)
build: image

test: image
	@log="$(BUILD)/$(TEST_NAME).log"; \
	simhlog="$(BUILD)/$(TEST_NAME).simh.log"; \
	echo "$(BOOT_KIND) $(TEST_NAME)"; \
	start=$$(date +%s); \
	timeout $(TIMEOUT) $(SIMH_PDP6) $(SIMH_INI) < /dev/null > "$$log" 2>&1; \
	rc=$$?; reason=; \
	if test "$$rc" -ne 0; then reason="SIMH exit $$rc"; fi; \
	if test -z "$$reason" && ! grep -F "CHANGE TAPE" "$$log" >/dev/null; then reason="missing CHANGE TAPE"; fi; \
	if test -z "$$reason" && ! grep -F "KERNEL" "$$log" >/dev/null; then reason="missing KERNEL"; fi; \
	if test -z "$$reason" && ! grep -F "DRIVERS" "$$log" >/dev/null; then reason="missing DRIVERS"; fi; \
	end=$$(date +%s); elapsed=$$((end - start)); \
	if test -n "$$reason"; then \
	  test -z "$$TEST_REPORT_DETAIL" || printf '%-6s %-4s %-18s %7ss  %s; log=%s\n' FAIL $(BOOT_KIND) $(TEST_NAME) "$$elapsed" "$$reason" "$$log" >> "$$TEST_REPORT_DETAIL"; \
	  echo "$(BOOT_KIND) $(TEST_NAME) failed: $$reason"; cat "$$log"; test ! -f "$$simhlog" || cat "$$simhlog"; exit 1; \
	fi; \
	test -z "$$TEST_REPORT_DETAIL" || printf '%-6s %-4s %-18s %7ss  two-tape handoff verified\n' PASS $(BOOT_KIND) $(TEST_NAME) "$$elapsed" >> "$$TEST_REPORT_DETAIL"

clean:
	rm -rf $(BUILD)

.DELETE_ON_ERROR:
