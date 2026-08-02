include ../bin/stat.mk

TAPE = $(BUILD)/mtc0.tap

.PHONY: all image test clean

all: image

$(TAPE): $(STAT_WORDS) $(BUILD)/mktap
	$(BUILD)/mktap -t mtc -p $(STAT_WORDS) -o $@

image: $(TAPE)

test: image
	@echo "mtc Stage1 reader is not implemented yet" >&2; exit 77

clean:
	rm -rf $(BUILD)
