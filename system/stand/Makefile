.DEFAULT_GOAL := build

BUILD_SUBDIRS = tools pdp6
TEST_SUBDIRS = pdp6
BUILD_ROOT ?= $(abspath ../../build)
TEST_REPORT ?= $(CURDIR)/test-report.txt

.PHONY: build test clean

build:
	@for d in $(BUILD_SUBDIRS); do \
		$(MAKE) -C $$d build BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)' || exit $$?; \
	done

test:
	@for d in $(TEST_SUBDIRS); do \
		$(MAKE) -C $$d test BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)' || exit $$?; \
	done

clean:
	rm -rf $(BUILD_ROOT)/system/stand test-report.txt test-report.txt.tmp test-report.txt.detail
