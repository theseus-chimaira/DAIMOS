.DEFAULT_GOAL := build

BUILD_ROOT ?= $(CURDIR)/build
TEST_REPORT ?= $(CURDIR)/test-report.txt

.PHONY: build install test kinit kinit-test clean

build:
	$(MAKE) -C system build BUILD_ROOT='$(BUILD_ROOT)'

install:
	$(MAKE) -C libc install BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

test:
	$(MAKE) -C tests test BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

kinit-test:
	$(MAKE) -C tests kinit-test BUILD_ROOT='$(BUILD_ROOT)'

clean:
	$(MAKE) -C libc clean BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	$(MAKE) -C tests clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT) $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).detail
