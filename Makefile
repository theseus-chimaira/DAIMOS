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

.PHONY: build tools install kinit boot aapboot disk-boot permanent-size clean

tools:
	$(MAKE) -C tools/host build BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

build: tools
	$(MAKE) -C system build BUILD_ROOT='$(BUILD_ROOT)'

install: tools
	$(MAKE) -C tools/host install BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C userland/libc install BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

boot: tools
	@printf '%s\n' 'DAIMOS PDP-6 login terminals:' \
	    '  CTY   current terminal' \
	    '  DCS0  127.0.0.1:$(SIMH_DCS0_PORT)' \
	    '  GE0   127.0.0.1:$(SIMH_GE0_PORT)' \
	    '  PCLK  $(SIMH_PCLK_MODE)'
	$(MAKE) -C system/boot/pdp6 boot BUILD_ROOT='$(BUILD_ROOT)' \
	    PDP10_PREFIX='$(PDP10_PREFIX)' SIMH_DCS0_PORT='$(SIMH_DCS0_PORT)' \
	    SIMH_GE0_PORT='$(SIMH_GE0_PORT)' SIMH_PCLK_MODE='$(SIMH_PCLK_MODE)' \
	    HOST_TOOLS='$(BUILD_ROOT)/tools/host' \
	    BOOT=dsk ROOT=disk ROOTSET='$(ROOTSET)'

aapboot: tools
	$(MAKE) -C system/boot/pdp6 image BOOT=dtc ROOT=tape ROOTSET='$(AAP_ROOTSET)' \
	    BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)' \
	    HOST_TOOLS='$(BUILD_ROOT)/tools/host'
	TMPDIR='$(TMPDIR)' scripts/aapboot-v1.sh \
	    '$(BUILD_ROOT)/system/boot/pdp6' \
	    '$(AAP_EMULATOR)' '$(AAP_CONFIG)' \
	    '$(BUILD_ROOT)/tools/host/dta2dtr' '$(AAP_ROOTSET)'

disk-boot:
	$(MAKE) boot ROOTSET='$(ROOTSET)'

permanent-size:
	$(MAKE) -C system/boot/pdp6 permanent-size BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'

clean:
	$(MAKE) -C tools/host clean BUILD_ROOT='$(BUILD_ROOT)'
	$(MAKE) -C userland/libc clean BUILD_ROOT='$(BUILD_ROOT)' PDP10_PREFIX='$(PDP10_PREFIX)'
	$(MAKE) -C system clean BUILD_ROOT='$(BUILD_ROOT)'
	rm -rf $(BUILD_ROOT)
