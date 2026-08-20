.DEFAULT_GOAL := build

SUBDIRS = system
BUILD_ROOT ?= $(CURDIR)/build
TEST_REPORT ?= $(CURDIR)/test-report.txt

.PHONY: build test kinit clean

build test:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@ BUILD_ROOT='$(BUILD_ROOT)' TEST_REPORT='$(TEST_REPORT)' || exit $$?; \
	done

kinit:
	$(MAKE) -C system kinit BUILD_ROOT='$(BUILD_ROOT)'

clean:
	rm -rf build test-report.txt test-report.txt.tmp test-report.txt.detail
	find system/stand/pdp6/test -type d -name build -prune -exec rm -rf {} +
