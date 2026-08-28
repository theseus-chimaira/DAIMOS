.DEFAULT_GOAL := build

BUILD_ROOT ?= $(CURDIR)/build
PDP10_PREFIX ?= $(HOME)/cross

.PHONY: build install kinit clean

build:
	$(MAKE) -C system build BUILD_ROOT='$(BUILD_ROOT)'

install:
	$(MAKE) -C libc install BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

clean:
	$(MAKE) -C libc clean BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT)
