.DEFAULT_GOAL := build

BUILD_ROOT ?= $(CURDIR)/build
PDP10_PREFIX ?= $(HOME)/cross
SIMH_DCS0_PORT ?= 1101
SIMH_GE0_PORT ?= 1201
SIMH_PCLK_MODE ?= REALTIME

.PHONY: build install kinit boot permanent-size clean

build:
	$(MAKE) -C system build BUILD_ROOT='$(BUILD_ROOT)'

install:
	$(MAKE) -C userland/libc install BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

boot:
	@printf '%s\n' 'DAIMOS PDP-6 login terminals:' \
	    '  CTY   current terminal' \
	    '  DCS0  127.0.0.1:$(SIMH_DCS0_PORT)' \
	    '  GE0   127.0.0.1:$(SIMH_GE0_PORT)' \
	    '  PCLK  $(SIMH_PCLK_MODE)'
	$(MAKE) -C system/boot/pdp6-disk boot BUILD_ROOT='$(BUILD_ROOT)' \
	    PDP10_PREFIX='$(PDP10_PREFIX)' SIMH_DCS0_PORT='$(SIMH_DCS0_PORT)' \
	    SIMH_GE0_PORT='$(SIMH_GE0_PORT)' SIMH_PCLK_MODE='$(SIMH_PCLK_MODE)'

permanent-size:
	$(MAKE) -C system/boot/pdp6-disk permanent-size BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

clean:
	$(MAKE) -C userland/libc clean BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT)
