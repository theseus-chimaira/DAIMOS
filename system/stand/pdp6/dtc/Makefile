# Production PDP-6 DECtape Stage-1 read-in image. Isolated tests may relocate
# their copy, but the standalone artifact matches the real Stage0 load address.
PROJECT_ROOT != cd ../../../.. && pwd
BUILD ?= ${PROJECT_ROOT}/build/system/stand/pdp6/dtc
PDP10_PREFIX ?= ${HOME}/cross
DAS ?= ${PDP10_PREFIX}/bin/das
DASFLAGS ?= -F
DXRCONVERT ?= ${PDP10_PREFIX}/bin/dxrconvert
STAGE1 = ${PROJECT_ROOT}/system/stand/pdp6/dtc/stage1.s
STAGE1_COMMON = ${PROJECT_ROOT}/system/stand/pdp6/common/decompressor.inc \
	${PROJECT_ROOT}/system/stand/pdp6/common/stage1-error.inc
STAGE1_DXR = ${BUILD}/stage1-dtc-v1.dxr
STAGE1_TAPE = ${BUILD}/stage1-dtc-v1.pt
STAGE1_BASE = 060

.PHONY: build clean

build: ${STAGE1_TAPE}

${BUILD}:
	mkdir -p $@

${STAGE1_DXR}: ${STAGE1} ${STAGE1_COMMON} ${BUILD}
	${DAS} ${DASFLAGS} -K -O $@ $<

${STAGE1_TAPE}: ${STAGE1_DXR}
	${DXRCONVERT} --pdp6-readin -b ${STAGE1_BASE} $< $@

clean:
	rm -rf ${BUILD}
