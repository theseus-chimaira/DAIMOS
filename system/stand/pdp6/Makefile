.DEFAULT_GOAL := build

BUILD_SUBDIRS = dsk dtc mtc ptr
TEST_SUBDIRS = test
BUILD_ROOT ?= $(abspath ../../../build)
BUILD ?= $(BUILD_ROOT)/system/stand/pdp6/test
TEST_REPORT ?= $(CURDIR)/test-report.txt

.PHONY: build test clean

build:
	@for d in $(BUILD_SUBDIRS); do \
		$(MAKE) -C $$d build BUILD=$(BUILD)/$$d || exit $$?; \
	done

test:
	@for d in $(TEST_SUBDIRS); do \
		$(MAKE) -C $$d test BUILD='$(BUILD)' TEST_REPORT='$(TEST_REPORT)' || exit $$?; \
	done

clean:
	rm -rf $(BUILD_ROOT)/system/stand/pdp6 test-report.txt test-report.txt.tmp test-report.txt.detail
