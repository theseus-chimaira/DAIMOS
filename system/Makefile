.DEFAULT_GOAL := build

BUILD_SUBDIRS = stand
BUILD_ROOT ?= $(abspath ../build)

.PHONY: build kinit clean

build:
	@for d in $(BUILD_SUBDIRS); do \
		$(MAKE) -C $$d build BUILD_ROOT='$(BUILD_ROOT)' || exit $$?; \
	done

kinit:
	$(MAKE) -C kernel kinit BUILD_ROOT='$(BUILD_ROOT)'

clean:
	rm -rf $(BUILD_ROOT)/system
