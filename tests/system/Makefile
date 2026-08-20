PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
TEST_REPORT ?= $(PROJECT_ROOT)/test-report.txt

.DEFAULT_GOAL := test

.PHONY: build test kinit-test clean

build:
	$(MAKE) -C stand build BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'

test:
	$(MAKE) -C stand test BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'
	$(MAKE) -C kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel'

kinit-test:
	$(MAKE) -C kernel test BUILD='$(BUILD_ROOT)/tests/system/kernel'

clean:
	$(MAKE) -C stand clean BUILD_ROOT='$(BUILD_ROOT)'
	$(MAKE) -C kernel clean BUILD='$(BUILD_ROOT)/tests/system/kernel'
