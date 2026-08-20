.DEFAULT_GOAL := build

BUILD_ROOT ?= $(CURDIR)/build
TEST_REPORT ?= $(CURDIR)/test-report.txt

.PHONY: build test kinit kinit-test clean

build:
	$(MAKE) -C system build BUILD_ROOT='$(BUILD_ROOT)'

test:
	$(MAKE) -C tests test BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

kinit-test:
	$(MAKE) -C tests kinit-test BUILD_ROOT='$(BUILD_ROOT)'

clean:
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	$(MAKE) -C tests clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT) $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).detail
