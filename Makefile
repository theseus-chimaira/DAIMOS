.DEFAULT_GOAL := build

BUILD_ROOT ?= $(CURDIR)/build
PDP10_PREFIX ?= $(HOME)/cross

.PHONY: build install kinit boot permanent-size clean

build:
	$(MAKE) -C system build BUILD_ROOT='$(BUILD_ROOT)'

install:
	$(MAKE) -C userland/libc install BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

boot:
	$(MAKE) -C system/boot/pdp6-disk boot BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

permanent-size:
	$(MAKE) -C system/boot/pdp6-disk permanent-size BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

clean:
	$(MAKE) -C userland/libc clean BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT)
