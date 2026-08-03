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

.PHONY: all image test clean

all: image

$(STAGE1_DXR): $(STAGE1) | $(BUILD)
	$(DAS) --base-kernel -o $@ $(STAGE1)

$(BUILD)/dxr2pdp6rim-v1: $(TOOLS_DIR)/dxr2pdp6rim_v1.c | $(BUILD)
	$(HOST_CC) -std=c89 -Wall -Wextra -O2 -o $@ $(TOOLS_DIR)/dxr2pdp6rim_v1.c

$(STAGE1_TAPE): $(STAGE1_DXR) $(BUILD)/dxr2pdp6rim-v1
	$(BUILD)/dxr2pdp6rim-v1 -b $(STAGE1_ENTRY) $(STAGE1_DXR) $@

image: $(STAT_WORDS) $(BUILD)/mkdsk $(STAGE1_TAPE)
	@for n in $(DSK_SETS); do \
	  for m in $(DSK_MODES); do \
	    out="$(BUILD)/dsk$${n}-$${m}"; \
	    rm -rf "$$out"; \
	    $(BUILD)/mkdsk -n "$$n" -m "$$m" -p $(STAT_WORDS) -o "$$out" || exit 1; \
	  done; \
	done

test: image
	@for n in $(DSK_SETS); do \
	  for m in $(DSK_MODES); do \
	    name="dsk$${n}-$${m}"; \
	    dofile="$(BUILD)/$$name.ini"; \
	    log="$(BUILD)/$$name.log"; \
	    echo "DSK $$name"; \
	    sed -e 's|@SIMH_LOG@|$(BUILD_ABS)/'"$$name"'.simh.log|g' \
	        -e 's|@STAGE1_TAPE@|$(STAGE1_TAPE_ABS)|g' \
	        $(SIMH_TEMPLATE) > "$$dofile"; \
	    i=0; \
	    while test "$$i" -lt "$$n"; do \
	      echo "attach -q dsk$$i $(BUILD_ABS)/$$name/dsk$$i.dsk" >> "$$dofile"; \
	      i=`expr $$i + 1`; \
	    done; \
	    echo "go 020" >> "$$dofile"; \
	    echo "exit" >> "$$dofile"; \
	    timeout $(TIMEOUT) $(SIMH_PDP6) "$$dofile" < /dev/null > "$$log" 2>&1; \
	    rc=$$?; \
	    if test "$$rc" -ne 0; then \
	      echo "SIMH failed for $$name: exit $$rc"; \
	      echo "command: timeout $(TIMEOUT) $(SIMH_PDP6) $$dofile < /dev/null"; \
	      cat "$$log"; \
	      cat "$(BUILD_ABS)/$$name.simh.log"; \
	      exit 1; \
	    fi; \
	    grep -F "BOOTLOADER OK" "$$log" >/dev/null || { cat "$$log"; exit 1; }; \
	    i=0; \
	    while test "$$i" -lt "$$n"; do \
	      grep -F "DSK$${i}  READ" "$$log" >/dev/null || { cat "$$log"; exit 1; }; \
	      i=`expr $$i + 1`; \
	    done; \
	  done; \
	done

clean:
	rm -rf $(BUILD)
