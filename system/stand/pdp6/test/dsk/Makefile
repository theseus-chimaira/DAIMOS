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

.PHONY: build image test test-positive test-negative test-extended clean

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

test: test-positive test-negative test-extended

test-positive: image
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



test-extended:
	@echo "DSK extended bad-run payload"
	@$(MAKE) test-positive \
	    BUILD='$(BUILD)/extended' \
	    STAT_BUILD='$(BUILD)/extended-stat' \
	    DSK_MODES=bad STAT_INIT_WORDS=02000

test-negative: image
	@set -e; \
	run_fail() { \
	  name="$$1"; expected="$$2"; attach="$$3"; disk="$$4"; \
	  dofile="$(BUILD)/$$name.ini"; \
	  log="$(BUILD)/$$name.log"; \
	  simhlog="$(BUILD)/$$name.simh.log"; \
	  sed -e 's|@SIMH_LOG@|$(BUILD_ABS)/'"$$name"'.simh.log|g' \
	      -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE_ABS)|g' \
	      $(SIMH_TEMPLATE) > "$$dofile"; \
	  if test -n "$$attach"; then \
	    echo "attach -q $$attach $$disk" >> "$$dofile"; \
	  fi; \
	  echo 'go 020' >> "$$dofile"; \
	  echo 'exit' >> "$$dofile"; \
	  echo "DSK $$name"; \
	  start=$$(date +%s); \
	  set +e; \
	  timeout $(TIMEOUT) $(SIMH_PDP6) "$$dofile" < /dev/null > "$$log" 2>&1; \
	  rc=$$?; \
	  set -e; \
	  reason=; \
	  if test "$$rc" -ne 0; then reason="SIMH exit $$rc"; fi; \
	  if test -z "$$reason" && ! grep -F "$$expected" "$$log" >/dev/null; then \
	    reason="missing $$expected"; \
	  fi; \
	  if test -z "$$reason" && ! grep -F "HALT instruction" "$$log" >/dev/null; then \
	    reason="missing HALT"; \
	  fi; \
	  if test -z "$$reason" && grep -F "BOOTLOADER OK" "$$log" >/dev/null; then \
	    reason="unexpected BOOTLOADER OK"; \
	  fi; \
	  end=$$(date +%s); elapsed=$$((end - start)); \
	  if test -n "$$reason"; then \
	    test -z "$$TEST_REPORT_DETAIL" || \
	      printf '%-6s %-4s %-18s %7ss  %s; log=%s\n' FAIL DSK "$$name" "$$elapsed" "$$reason" "$$log" >> "$$TEST_REPORT_DETAIL"; \
	    echo "DSK $$name failed: $$reason"; \
	    cat "$$log"; \
	    test ! -f "$$simhlog" || cat "$$simhlog"; \
	    exit 1; \
	  fi; \
	  test -z "$$TEST_REPORT_DETAIL" || \
	    printf '%-6s %-4s %-18s %7ss  expected failure %s\n' PASS DSK "$$name" "$$elapsed" "$$expected" >> "$$TEST_REPORT_DETAIL"; \
	}; \
	run_fail dsk-neg-nodsk '?NODSK' '' ''; \
	run_fail dsk-neg-noset '?NOSET' dsk0 "$(BUILD_ABS)/dsk2-clean/dsk0.dsk"; \
	cp "$(BUILD_ABS)/dsk1-clean/dsk0.dsk" "$(BUILD_ABS)/dsk-neg-khead.dsk"; \
	dd if=/dev/zero of="$(BUILD_ABS)/dsk-neg-khead.dsk" bs=8 seek=128 count=1 conv=notrunc >/dev/null 2>&1; \
	run_fail dsk-neg-khead '?KHEAD' dsk0 "$(BUILD_ABS)/dsk-neg-khead.dsk"; \
	cp "$(BUILD_ABS)/dsk1-clean/dsk0.dsk" "$(BUILD_ABS)/dsk-neg-layout.dsk"; \
	dd if=/dev/zero of="$(BUILD_ABS)/dsk-neg-layout.dsk" bs=8 seek=129 count=1 conv=notrunc >/dev/null 2>&1; \
	run_fail dsk-neg-layout '?LAYOT' dsk0 "$(BUILD_ABS)/dsk-neg-layout.dsk"

clean:
	rm -rf $(BUILD)
