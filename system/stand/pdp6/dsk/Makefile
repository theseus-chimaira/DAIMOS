PROJECT_ROOT := $(abspath ../../../..)
BUILD ?= $(PROJECT_ROOT)/build/tests/system/stand/pdp6/dsk
TEST_DIR := $(PROJECT_ROOT)/tests/system/stand/pdp6/dsk

.DEFAULT_GOAL := build

.PHONY: build image test clean

build image test:
	$(MAKE) -C $(TEST_DIR) $@ BUILD='$(BUILD)'

clean:
	rm -rf $(BUILD)
