.DEFAULT_GOAL := build

BUILD_ROOT ?= $(CURDIR)/build
PDP10_PREFIX ?= $(HOME)/cross
TMPDIR ?= $(HOME)/tmp
AAP_EMULATOR ?= pdp6-aap-v2
AAP_CONFIG ?= /usr/local/share/pdp6-aap-v2/init-full-8dt-v2.ini
AAP_ROOTSET ?= $(ROOTSET)
SIMH_DCS0_PORT ?= 1101
SIMH_GE0_PORT ?= 1201
SIMH_PCLK_MODE ?= REALTIME
ROOT ?= auto
ROOTSET ?= 1

.PHONY: build install kinit boot aapboot disk-boot permanent-size clean

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
	$(MAKE) -C system/boot/pdp6 boot BUILD_ROOT='$(BUILD_ROOT)' \
	    PDP10_PREFIX='$(PDP10_PREFIX)' SIMH_DCS0_PORT='$(SIMH_DCS0_PORT)' \
	    SIMH_GE0_PORT='$(SIMH_GE0_PORT)' SIMH_PCLK_MODE='$(SIMH_PCLK_MODE)' \
	    ROOT='$(ROOT)' ROOTSET='$(ROOTSET)'

aapboot:
	$(MAKE) -C system/boot/pdp6 image BOOT=dtc ROOT=tape ROOTSET='$(AAP_ROOTSET)' \
	    BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	TMPDIR='$(TMPDIR)' scripts/aapboot-v1.sh \
	    '$(BUILD_ROOT)/system/boot/pdp6' \
	    '$(AAP_EMULATOR)' '$(AAP_CONFIG)' \
	    '$(PDP10_PREFIX)/bin/dta2dtr' '$(AAP_ROOTSET)'

disk-boot:
	$(MAKE) boot ROOT='$(ROOT)' ROOTSET='$(ROOTSET)'

permanent-size:
	$(MAKE) -C system/boot/pdp6 permanent-size BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

clean:
	$(MAKE) -C userland/libc clean BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT)
