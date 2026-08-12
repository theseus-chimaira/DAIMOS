PROJECT_ROOT := $(abspath ../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/dtc

.DEFAULT_GOAL := build

.PHONY: build image test clean

build image test:
	$(MAKE) -C ../test/dtc $@ BUILD='$(BUILD)'

clean:
	rm -rf $(BUILD) test-report.txt test-report.txt.tmp test-report.txt.detail
