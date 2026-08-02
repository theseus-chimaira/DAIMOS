.PHONY: test dsk ptr mtc dtc clean

test: dsk ptr mtc dtc

dsk:
	$(MAKE) -C dsk test

ptr:
	$(MAKE) -C ptr test

mtc:
	$(MAKE) -C mtc test

dtc:
	$(MAKE) -C dtc test

clean:
	$(MAKE) -C dsk clean
	$(MAKE) -C ptr clean
	$(MAKE) -C mtc clean
	$(MAKE) -C dtc clean
