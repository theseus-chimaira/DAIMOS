BUILD_SUBDIRS = dsk drm dtc mtc ptr
PDP6_STAND_DIR != pwd
BUILD_ROOT ?= ${PDP6_STAND_DIR}/../../../build
BUILD ?= ${BUILD_ROOT}/system/stand/pdp6
PDP10_PREFIX ?= ${HOME}/cross

.PHONY: build clean

build:
	@for d in ${BUILD_SUBDIRS}; do \
		${MAKE} -C $$d build BUILD='${BUILD}/'$$d PDP10_PREFIX='${PDP10_PREFIX}' || exit $$?; \
	done

clean:
	rm -rf ${BUILD}
