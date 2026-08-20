PROJECT_ROOT := $(abspath ../../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
TEST_REPORT ?= $(PROJECT_ROOT)/test-report.txt

.DEFAULT_GOAL := test

.PHONY: build test clean

build test:
	$(MAKE) -C pdp6 $@ BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)'

clean:
	$(MAKE) -C pdp6 clean BUILD_ROOT='$(BUILD_ROOT)'
