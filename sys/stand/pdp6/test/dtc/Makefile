include ../bin/stat.mk

TAPE = $(BUILD)/dtc0.dt

.PHONY: all image test clean

all: image

$(TAPE): $(STAT_WORDS) $(BUILD)/mkdt
	$(BUILD)/mkdt -t dtc -p $(STAT_WORDS) -o $@

image: $(TAPE)

test: image
	@echo "dtc Stage1 reader is not implemented yet" >&2; exit 77

clean:
	rm -rf $(BUILD)
