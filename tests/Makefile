PROJECT_ROOT := $(abspath ..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
TEST_REPORT ?= $(PROJECT_ROOT)/test-report.txt

.DEFAULT_GOAL := test

.PHONY: build test kinit-test clean

build test:
	$(MAKE) -C system $@ BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'

kinit-test:
	$(MAKE) -C system/kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel'

clean:
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	$(MAKE) -C simh clean
