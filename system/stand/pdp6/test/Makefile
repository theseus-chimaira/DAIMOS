PROJECT_ROOT := $(abspath $(CURDIR)/../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test
TEST_REPORT ?= $(BUILD)/test-report.txt
TEST_REPORT_DETAIL = $(abspath $(TEST_REPORT).detail)

.PHONY: test test-inner dsk ptr mtc dtc clean

test:
	@mkdir -p $(BUILD)
	@rm -f $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).detail
	@start=$$(date +%s); started=$$(date); \
	TEST_REPORT_DETAIL='$(TEST_REPORT_DETAIL)' \
	  $(MAKE) --no-print-directory test-inner; rc=$$?; \
	end=$$(date +%s); elapsed=$$((end - start)); \
	passed=0; failed=0; \
	if test -f $(TEST_REPORT).detail; then \
	  passed=`awk '$$1 == "PASS" { n++ } END { print n + 0 }' $(TEST_REPORT).detail`; \
	  failed=`awk '$$1 == "FAIL" { n++ } END { print n + 0 }' $(TEST_REPORT).detail`; \
	fi; \
	{ \
	  echo "DAIMOS PDP-6 bootloader test report"; \
	  echo "====================================="; \
	  echo "started:  $$started"; \
	  echo "finished: `date`"; \
	  echo "commit:   `git -C ../../../.. rev-parse --short HEAD 2>/dev/null || echo unknown`"; \
	  echo "elapsed:  $${elapsed}s"; \
	  echo; \
	  printf '%-6s %-4s %-18s %8s  %s\n' RESULT TYPE TEST TIME DETAILS; \
	  printf '%-6s %-4s %-18s %8s  %s\n' ------ ---- ------------------ -------- -------; \
	  if test -s $(TEST_REPORT).detail; then cat $(TEST_REPORT).detail; \
	  else echo "FAIL   TEST no-tests-run              test driver stopped before recording results"; fi; \
	  echo; \
	  echo "summary: $$passed passed, $$failed failed"; \
	  if test $$rc -eq 0; then echo "result:  PASS"; else echo "result:  FAIL"; fi; \
	} > $(TEST_REPORT).tmp; \
	mv $(TEST_REPORT).tmp $(TEST_REPORT); \
	rm -f $(TEST_REPORT).detail; \
	exit $$rc

test-inner: dsk ptr mtc dtc

dsk:
	$(MAKE) -C dsk BUILD=$(BUILD)/dsk test

ptr:
	$(MAKE) -C ptr BUILD=$(BUILD)/ptr test

mtc:
	$(MAKE) -C mtc BUILD=$(BUILD)/mtc test

dtc:
	$(MAKE) -C dtc BUILD=$(BUILD)/dtc test

clean:
	$(MAKE) -C dsk BUILD=$(BUILD)/dsk clean
	$(MAKE) -C ptr BUILD=$(BUILD)/ptr clean
	$(MAKE) -C mtc BUILD=$(BUILD)/mtc clean
	$(MAKE) -C dtc BUILD=$(BUILD)/dtc clean
	rm -rf $(BUILD)
