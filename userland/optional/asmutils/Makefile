DAS ?= $(PDP10_PREFIX)/bin/das
DASFLAGS ?= -B -A
SIXMD_CHECK ?= $(PDP10_PREFIX)/bin/sixmd-check
S6FILTER ?= $(PDP10_PREFIX)/bin/s6filter
DESTDIR ?=
PREFIX ?= /OPTION/BASE

PROGRAMS = ARGS ASCII2SIX BENCH CPUSPEED CRC DIS DXRINFO ECHO FACTOR FALSE \
	MKDIR MV OD PRIME RAND REV RM RMDIR SIX2ASCII SLEEP SORT SUM TOUCH TR TRUE \
	UNIQ YES
DXRS = $(PROGRAMS:%=build/%.DXR)
MANUAL_SRCS = $(wildcard manual/*.SIXMD)
MANUAL_PAGES = build/manual/PAGES
MANUAL_INDEX = build/manual/INDEX

.PHONY: all clean install install-source check manual manual-check

all: manual-check $(DXRS)

build/%.DXR: %.S BASE.S S6REC.S
	mkdir -p build
	$(DAS) $(DASFLAGS) -O $@ $<

manual-check:
	@set -eu; for p in $(PROGRAMS); do \
		f="manual/$$p.SIXMD"; \
		test -f "$$f" || { echo "missing SIXMD manual for $$p" >&2; exit 1; }; \
		$(SIXMD_CHECK) "$$f" "$$p"; \
	done
	@set -e; for f in $(MANUAL_SRCS); do n=$$(basename "$$f" .SIXMD); $(SIXMD_CHECK) "$$f" "$$n"; done

check: all
	@set -e; for f in $(DXRS); do test -s "$$f"; done

manual: manual-check $(MANUAL_PAGES) $(MANUAL_INDEX)

$(MANUAL_PAGES): $(MANUAL_SRCS)
	mkdir -p $(dir $@)
	@set -eu; t=$@.txt; : > "$$t"; for f in $(MANUAL_SRCS); do cat "$$f" >> "$$t"; done; $(S6FILTER) "$$t" > $@; rm -f "$$t"

$(MANUAL_INDEX): $(MANUAL_SRCS)
	mkdir -p $(dir $@)
	@set -eu; t=$@.txt; : > "$$t"; for f in $(MANUAL_SRCS); do sed -n 's/^# @?//p' "$$f" | head -n 1 >> "$$t"; done; $(S6FILTER) "$$t" > $@; rm -f "$$t"

install: all manual
	mkdir -p $(DESTDIR)$(PREFIX)/EXEC
	@set -e; for p in $(PROGRAMS); do cp "build/$$p.DXR" "$(DESTDIR)$(PREFIX)/EXEC/$$p"; done
	mkdir -p $(DESTDIR)$(PREFIX)/MANUAL/ASMUTILS
	cp $(MANUAL_PAGES) $(DESTDIR)$(PREFIX)/MANUAL/ASMUTILS/PAGES
	cp $(MANUAL_INDEX) $(DESTDIR)$(PREFIX)/MANUAL/ASMUTILS/INDEX

install-source:
	mkdir -p $(DESTDIR)$(PREFIX)/SOURCE/ASMUTILS
	cp *.S MAKEFILE $(DESTDIR)$(PREFIX)/SOURCE/ASMUTILS/
	mkdir -p $(DESTDIR)$(PREFIX)/SOURCE/ASMUTILS/MANUAL
	cp manual/*.SIXMD $(DESTDIR)$(PREFIX)/SOURCE/ASMUTILS/MANUAL/

clean:
	rm -rf build
