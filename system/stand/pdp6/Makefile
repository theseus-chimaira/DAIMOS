.DEFAULT_GOAL := build

BUILD_SUBDIRS = dsk dtc mtc ptr
BUILD_ROOT ?= $(abspath ../../../build)
BUILD ?= $(BUILD_ROOT)/tests/system/stand/pdp6

.PHONY: build clean

build:
	@for d in $(BUILD_SUBDIRS); do \
		$(MAKE) -C $$d build BUILD='$(BUILD)/'$$d || exit $$?; \
	done

clean:
	rm -rf $(BUILD_ROOT)/system/stand/pdp6 $(BUILD_ROOT)/tests/system/stand/pdp6
