PROJECT_ROOT := $(abspath ../../../../..)
BUILD ?= $(PROJECT_ROOT)/build/system/stand/pdp6/test/dsk

include ../bin/stat.mk

DSK_SETS ?= 1 2 3 4
DSK_MODES ?= clean db0 db1 bad
STAGE1 = ../../dsk/stage1.s
STAGE1_ENTRY = 060000
STAGE1_DXR = $(BUILD)/stage1-dsk-v1.dxr
STAGE1_TAPE = $(BUILD)/stage1-dsk-v1.pt
BUILD_ABS = $(abspath $(BUILD))
STAGE1_TAPE_ABS = $(abspath $(STAGE1_TAPE))
SIMH_TEMPLATE = simh-v1.ini.in

.DEFAULT_GOAL := build

.PHONY: build image test clean

build: image

$(STAGE1_DXR): $(STAGE1) | $(BUILD)
	$(DAS) --base-kernel -o $@ $(STAGE1)

$(STAGE1_TAPE): $(STAGE1_DXR)
	$(DXRCONVERT) --pdp6-readin -b $(STAGE1_ENTRY) $(STAGE1_DXR) $@

image: $(STAT_WORDS) $(MKDSK) $(STAGE1_TAPE)
	@for n in $(DSK_SETS); do \
	  for m in $(DSK_MODES); do \
	    out="$(BUILD)/dsk$${n}-$${m}"; \
	    rm -rf "$$out"; \
	    $(MKDSK) -n "$$n" -m "$$m" -p $(STAT_WORDS) -o "$$out" || exit 1; \
	  done; \
	done

test: image
	@for n in $(DSK_SETS); do \
	  for m in $(DSK_MODES); do \
	    name="dsk$${n}-$${m}"; \
	    dofile="$(BUILD)/$$name.ini"; \
	    log="$(BUILD)/$$name.log"; \
	    echo "DSK $$name"; \
	    start=$$(date +%s); \
	    sed -e 's|@SIMH_LOG@|$(BUILD_ABS)/'"$$name"'.simh.log|g' \
	        -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE_ABS)|g' \
	        $(SIMH_TEMPLATE) > "$$dofile"; \
	    i=0; \
	    while test "$$i" -lt "$$n"; do \
	      echo "attach -q dsk$$i $(BUILD_ABS)/$$name/dsk$$i.dsk" >> "$$dofile"; \
	      i=`expr $$i + 1`; \
	    done; \
	    echo 'expect HALTAFTER=0 "TEST  DONE" exit' >> "$$dofile"; \
	    echo "go 020" >> "$$dofile"; \
	    echo "exit" >> "$$dofile"; \
	    timeout $(TIMEOUT) $(SIMH_PDP6) "$$dofile" < /dev/null > "$$log" 2>&1; \
	    rc=$$?; \
	    reason=; \
	    if test "$$rc" -ne 0; then reason="SIMH exit $$rc"; fi; \
	    if test -z "$$reason" && ! grep -F "BOOTLOADER OK" "$$log" >/dev/null; then \
	      reason="missing BOOTLOADER OK"; \
	    fi; \
	    i=0; \
	    while test -z "$$reason" && test "$$i" -lt "$$n"; do \
	      if ! grep -F "DSK$${i}  READ" "$$log" >/dev/null; then reason="missing DSK$${i} READ"; fi; \
	      i=`expr $$i + 1`; \
	    done; \
	    end=$$(date +%s); elapsed=$$((end - start)); \
	    if test -n "$$reason"; then \
	      test -z "$$TEST_REPORT_DETAIL" || \
	        printf '%-6s %-4s %-18s %7ss  %s; log=%s\n' FAIL DSK "$$name" "$$elapsed" "$$reason" "$$log" >> "$$TEST_REPORT_DETAIL"; \
	      echo "DSK $$name failed: $$reason"; \
	      cat "$$log"; \
	      cat "$(BUILD_ABS)/$$name.simh.log"; \
	      exit 1; \
	    fi; \
	    test -z "$$TEST_REPORT_DETAIL" || \
	      printf '%-6s %-4s %-18s %7ss  disks=%s mode=%s\n' PASS DSK "$$name" "$$elapsed" "$$n" "$$m" >> "$$TEST_REPORT_DETAIL"; \
	  done; \
	done

clean:
	rm -rf $(BUILD)
