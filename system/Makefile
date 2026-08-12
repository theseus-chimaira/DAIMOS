.DEFAULT_GOAL := build

SUBDIRS = stand
BUILD_ROOT ?= $(abspath ../build)
TEST_REPORT ?= $(CURDIR)/test-report.txt

.PHONY: build test clean

build test:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@ BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)' || exit $$?; \
	done

clean:
	rm -rf $(BUILD_ROOT)/system test-report.txt test-report.txt.tmp test-report.txt.detail
