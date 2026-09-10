BUILD_SUBDIRS = stand
SYSTEM_DIR != pwd
BUILD_ROOT ?= ${SYSTEM_DIR}/../build

.PHONY: build kinit clean

build:
	@for d in ${BUILD_SUBDIRS}; do \
		${MAKE} -C $$d build BUILD_ROOT='${BUILD_ROOT}' || exit $$?; \
	done

kinit:
	${MAKE} -C kernel kinit BUILD_ROOT='${BUILD_ROOT}'

clean:
	rm -rf ${BUILD_ROOT}/system
