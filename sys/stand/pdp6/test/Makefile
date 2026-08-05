TEST_REPORT ?= test-report.txt

.PHONY: test test-inner dsk ptr mtc dtc clean

test:
	@rm -f $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).status
	@start=`date +%s`; \
	{ \
	  echo "DAIMOS PDP-6 bootloader test report"; \
	  echo "====================================="; \
	  echo "started: `date`"; \
	  echo "commit:  `git -C ../../../.. rev-parse --short HEAD 2>/dev/null || echo unknown`"; \
	  echo "host:    `uname -a 2>/dev/null || echo unknown`"; \
	  echo; \
	  $(MAKE) --no-print-directory test-inner; rc=$$?; \
	  end=`date +%s`; elapsed=`expr $$end - $$start`; \
	  echo; \
	  echo "====================================="; \
	  echo "finished: `date`"; \
	  echo "elapsed:  $${elapsed}s"; \
	  if test $$rc -eq 0; then result=PASS; else result=FAIL; fi; \
	  echo "result:   $$result"; \
	  echo $$rc > $(TEST_REPORT).status; \
	} 2>&1 | tee $(TEST_REPORT).tmp; \
	rc=`cat $(TEST_REPORT).status`; \
	mv $(TEST_REPORT).tmp $(TEST_REPORT); \
	rm -f $(TEST_REPORT).status; \
	exit $$rc

test-inner: dsk ptr mtc dtc

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
	rm -f $(TEST_REPORT) $(TEST_REPORT).tmp $(TEST_REPORT).status
