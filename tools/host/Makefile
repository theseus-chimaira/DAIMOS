PROJECT_ROOT := $(abspath ../..)
BUILD_ROOT ?= $(PROJECT_ROOT)/build
BUILD := $(BUILD_ROOT)/tools/host
PDP10_PREFIX ?= $(HOME)/cross
DESTDIR ?=
CC ?= cc
CFLAGS ?= -O2
PKG_CONFIG ?= pkg-config
MAGICKWAND_CFLAGS ?= $(shell $(PKG_CONFIG) --cflags MagickWand)
MAGICKWAND_LIBS ?= $(shell $(PKG_CONFIG) --libs MagickWand)

TOOLS = d6lz mkdsk mkd6fs mkauxstore d6fsck packfs d6bad logstore d6swap \
	mkdt dta2dtr mktsfs tsfscheck mkstream mktap words2pt dlink darc \
	p10run p10job pdp10-objdump p10fold p10super mkinitfs0 mkbootbanner \
	sixmd-check csix img2dpic
PROGRAMS = $(addprefix $(BUILD)/,$(TOOLS))
ALIASES = pdp10-dec-none-darc pdp10-dec-none-objdump mkinitfs
STALE_TOOLS = pdp10-dec-none-ar pdp10-dec-none-ranlib dxr2rim mkrim mkrim.py \
	p10bare p10bare.py
INSTALL_ROOT = $(DESTDIR)$(PDP10_PREFIX)

.DEFAULT_GOAL := build
.PHONY: all build install uninstall clean

all: build
build: $(PROGRAMS)

$(BUILD):
	mkdir -p $@

$(BUILD)/d6lz: d6lz.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ d6lz.c

$(BUILD)/mkdsk: mkdsk.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mkdsk.c

$(BUILD)/mkd6fs: mkd6fs.c d6maint.c d6maint.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mkd6fs.c d6maint.c

$(BUILD)/mkauxstore: mkauxstore.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ mkauxstore.c

$(BUILD)/d6fsck: d6fsck.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ d6fsck.c

$(BUILD)/packfs: packfs.c d6maint.c d6maint.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ packfs.c d6maint.c

$(BUILD)/d6bad: d6bad.c d6maint.c d6maint.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ d6bad.c d6maint.c

$(BUILD)/logstore: logstore.c d6maint.c d6maint.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ logstore.c d6maint.c

$(BUILD)/d6swap: d6swap.c d6maint.c d6maint.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ d6swap.c d6maint.c

$(BUILD)/mkdt: mkdt.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mkdt.c

$(BUILD)/dta2dtr: dta2dtr.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ dta2dtr.c

$(BUILD)/mktsfs: mktsfs.c tsfs-format.c tsfs-format.h d6lz-codec.c d6lz-codec.h | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -o $@ mktsfs.c tsfs-format.c d6lz-codec.c

$(BUILD)/tsfscheck: tsfscheck.c tsfs-format.c tsfs-format.h d6lz-codec.c d6lz-codec.h | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -o $@ tsfscheck.c tsfs-format.c d6lz-codec.c

$(BUILD)/mkstream: mkstream.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mkstream.c

$(BUILD)/mktap: mktap.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mktap.c

$(BUILD)/words2pt: words2pt.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ words2pt.c

$(BUILD)/dlink: dlink.c dobj.c dobj.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ dlink.c dobj.c

$(BUILD)/darc: darc.c dobj.c dobj.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ darc.c dobj.c

$(BUILD)/p10run: p10run.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ p10run.c

$(BUILD)/p10job: p10job.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ p10job.c

$(BUILD)/pdp10-objdump: pdp10-objdump.c dobj.c dobj.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ pdp10-objdump.c dobj.c

$(BUILD)/p10fold: p10fold.c dobj.c dobj.h | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ p10fold.c dobj.c

$(BUILD)/p10super: p10super.c dobj.c dobj.h | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ p10super.c dobj.c

$(BUILD)/mkinitfs0: mkinitfs0.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mkinitfs0.c

$(BUILD)/mkbootbanner: mkbootbanner.c | $(BUILD)
	$(CC) $(CFLAGS) -o $@ mkbootbanner.c

$(BUILD)/sixmd-check: sixmd-check.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ sixmd-check.c

$(BUILD)/csix: csix.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror -o $@ csix.c

$(BUILD)/img2dpic: img2dpic.c | $(BUILD)
	$(CC) $(CFLAGS) -std=c99 -Wall -Wextra -Werror $(MAGICKWAND_CFLAGS) \
		-o $@ img2dpic.c $(MAGICKWAND_LIBS)

install: build
	mkdir -p $(INSTALL_ROOT)/bin $(INSTALL_ROOT)/include \
		$(INSTALL_ROOT)/share/daimos/simh
	@for f in $(STALE_TOOLS); do rm -f $(INSTALL_ROOT)/bin/$$f; done
	cp $(PROGRAMS) $(INSTALL_ROOT)/bin/
	ln -sf darc $(INSTALL_ROOT)/bin/pdp10-dec-none-darc
	ln -sf pdp10-objdump $(INSTALL_ROOT)/bin/pdp10-dec-none-objdump
	ln -sf mkinitfs0 $(INSTALL_ROOT)/bin/mkinitfs
	cp pdp10-sixbit.h tsfs-format.h $(INSTALL_ROOT)/include/
	cp simh/*.ini $(INSTALL_ROOT)/share/daimos/simh/

uninstall:
	@for f in $(TOOLS) $(ALIASES) $(STALE_TOOLS); do rm -f $(INSTALL_ROOT)/bin/$$f; done
	rm -f $(INSTALL_ROOT)/include/pdp10-sixbit.h $(INSTALL_ROOT)/include/tsfs-format.h
	@for f in simh/*.ini; do rm -f $(INSTALL_ROOT)/share/daimos/simh/$${f##*/}; done

clean:
	rm -rf $(BUILD)
