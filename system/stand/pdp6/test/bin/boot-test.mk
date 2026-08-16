# Common Stage1 build and SIMH test rules for single-device boot loaders.

.DEFAULT_GOAL := build

.PHONY: build image test clean

build: image

$(STAGE1_DXR): $(STAGE1) $(STAGE1_COMMON_DEPS) | $(BUILD)
	$(DAS) --base-kernel -o $@ $(STAGE1)

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b $(STAGE1_ENTRY) $(STAGE1_DXR) $@

$(SIMH_INI): $(SIMH_TEMPLATE) $(STAGE1_TAPE) $(PAYLOAD_TAPE) | $(BUILD)
	sed -e 's|@SIMH_LOG@|$(BUILD)/$(TEST_NAME).simh.log|g' \
	    -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE)|g' \
	    -e 's|@PAYLOAD_TAPE@|$(PAYLOAD_TAPE)|g' \
	    $(SIMH_SED_EXTRA) \
	    $(SIMH_TEMPLATE) > $@

image: $(STAGE1_TAPE) $(PAYLOAD_TAPE) $(SIMH_INI)

test: image
	@log="$(BUILD)/$(TEST_NAME).log"; \
	simhlog="$(BUILD)/$(TEST_NAME).simh.log"; \
	echo "$(BOOT_KIND) $(TEST_NAME)"; \
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
	      printf '%-6s %-4s %-18s %7ss  %s; log=%s\n' FAIL $(BOOT_KIND) $(TEST_NAME) "$$elapsed" "$$reason" "$$log" >> "$$TEST_REPORT_DETAIL"; \
	    echo "$(BOOT_KIND) $(TEST_NAME) failed: $$reason"; \
	    cat "$$log"; \
	    test ! -f "$$simhlog" || cat "$$simhlog"; \
	    exit 1; \
	fi; \
	test -z "$$TEST_REPORT_DETAIL" || \
	  printf '%-6s %-4s %-18s %7ss  bootloader and payload verified\n' PASS $(BOOT_KIND) $(TEST_NAME) "$$elapsed" >> "$$TEST_REPORT_DETAIL"

clean:
	rm -rf $(BUILD)
