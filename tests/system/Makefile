PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
TEST_REPORT ?= $(PROJECT_ROOT)/test-report.txt

.DEFAULT_GOAL := test

.PHONY: build test kinit-test clean

build test:
	$(MAKE) -C stand $@ BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'

kinit-test:
	$(MAKE) -C kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel'

clean:
	$(MAKE) -C stand clean BUILD_ROOT='$(BUILD_ROOT)'
	$(MAKE) -C kernel clean BUILD='$(BUILD_ROOT)/tests/system/kernel'
