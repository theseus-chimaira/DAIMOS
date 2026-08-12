PROJECT_ROOT := $(abspath ../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/ptr

.DEFAULT_GOAL := build

.PHONY: build image test clean

build image test:
	$(MAKE) -C ../test/ptr $@ BUILD='$(BUILD)'

clean:
	rm -rf $(BUILD) test-report.txt test-report.txt.tmp test-report.txt.detail
