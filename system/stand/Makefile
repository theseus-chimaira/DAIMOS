.DEFAULT_GOAL := build

BUILD_SUBDIRS = pdp6
BUILD_ROOT ?= $(abspath ../../build)

.PHONY: build clean

build:
	@for d in $(BUILD_SUBDIRS); do \
		$(MAKE) -C $$d build BUILD_ROOT='$(BUILD_ROOT)' || exit $$?; \
	done

clean:
	rm -rf $(BUILD_ROOT)/system/stand
