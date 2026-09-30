# Top-level standalone/bootstrap build dispatcher. Machine-specific standalone
# programs live below system/stand/<machine>; this file only fans out selected
# architectures and keeps their generated files below BUILD_ROOT.
BUILD_SUBDIRS = pdp6
STAND_DIR != pwd
BUILD_ROOT ?= ${STAND_DIR}/../../build

.PHONY: build clean

build:
	@for d in ${BUILD_SUBDIRS}; do \
		${MAKE} -C $$d build BUILD_ROOT='${BUILD_ROOT}' || exit $$?; \
	done

clean:
	rm -rf ${BUILD_ROOT}/system/stand
