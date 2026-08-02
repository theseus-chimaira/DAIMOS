include ../bin/stat.mk

DSK_SETS ?= 1 2 4
DSK_MODES ?= clean db0 db1 bad
STAGE1 = ../../dsk/stage1.s
STAGE1_RIM = $(BUILD)/stage1-dsk.rim

.PHONY: all image test clean

all: image

$(STAGE1_RIM): $(STAGE1) | $(BUILD)
	$(PDP10_AS) --start 074000 $(STAGE1) > $@

image: $(STAT_WORDS) $(BUILD)/mkdsk $(STAGE1_RIM)
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
	    { \
	      echo "set -q -n log $(BUILD)/$$name.simh.log"; \
	      echo "set cpu 32k"; \
	      cat ../simh/off.ini; \
	      echo "set dsk enabled"; \
	      i=0; \
	      while test "$$i" -lt "$$n"; do \
	        echo "attach -q dsk$$i $(BUILD)/$$name/dsk$$i.dsk"; \
	        i=`expr $$i + 1`; \
	      done; \
	      echo "do $(STAGE1_RIM)"; \
	      echo "exit"; \
	    } > "$$dofile"; \
	    $(SIMH_PDP6) "$$dofile" > "$$log" 2>&1 || { cat "$$log"; exit 1; }; \
	    grep -F "STAGE1 OK" "$$log" >/dev/null || { cat "$$log"; exit 1; }; \
	    i=0; \
	    while test "$$i" -lt "$$n"; do \
	      grep -F "DSK$${i}  READ" "$$log" >/dev/null || { cat "$$log"; exit 1; }; \
	      i=`expr $$i + 1`; \
	    done; \
	  done; \
	done

clean:
	rm -rf $(BUILD)
