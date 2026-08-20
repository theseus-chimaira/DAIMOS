PROJECT_ROOT := $(abspath ../../../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD ?= $(BUILD_ROOT)/tests/system/stand/pdp6
TEST_REPORT ?= $(PROJECT_ROOT)/test-report.txt
TEST_REPORT_DETAIL := $(abspath $(TEST_REPORT).detail)
SUBDIRS = dsk ptr mtc dtc

.DEFAULT_GOAL := build

.PHONY: build test test_inner clean

build:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d BUILD='$(BUILD)/'$$d build || exit $$?; \
	done

test:
	@mkdir -p $(BUILD) $(dir $(TEST_REPORT))
	@rm -f $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).detail
	@start=$$(date +%s); started=$$(date); \
	TEST_REPORT_DETAIL='$(TEST_REPORT_DETAIL)' \
	  $(MAKE) --no-print-directory test_inner; rc=$$?; \
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
	  echo "commit:   `git -C $(PROJECT_ROOT) rev-parse --short HEAD 2>/dev/null || echo unknown`"; \
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

test_inner:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d BUILD='$(BUILD)/'$$d test || exit $$?; \
	done

clean:
	rm -rf $(BUILD) $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).detail
