# Production PDP-6 DSK270 Stage-1 read-in image, loaded by Stage0 at 000060.
PROJECT_ROOT != cd ../../../.. && pwd
BUILD ?= ${PROJECT_ROOT}/build/system/stand/pdp6/dsk
PDP10_PREFIX ?= ${HOME}/cross
DAS ?= das
DASFLAGS ?= -F
DXRCONVERT ?= dxrconvert
STAGE1 = ${PROJECT_ROOT}/system/stand/pdp6/dsk/stage1.s
STAGE1_COMMON = ${PROJECT_ROOT}/system/stand/pdp6/common/decompressor.inc \
	${PROJECT_ROOT}/system/stand/pdp6/common/stage1-error.inc
STAGE1_DXR = ${BUILD}/stage1-dsk-v1.dxr
STAGE1_TAPE = ${BUILD}/stage1-dsk-v1.pt
STAGE1_BASE = 060

.PHONY: build clean

build: ${STAGE1_TAPE}

${BUILD}:
	mkdir -p $@

${STAGE1_DXR}: ${STAGE1} ${STAGE1_COMMON} ${BUILD}
	${DAS} ${DASFLAGS} -K -O $@ $<

${STAGE1_TAPE}: ${STAGE1_DXR}
	${DXRCONVERT} --pdp6-readin -b ${STAGE1_BASE} $< $@
	printf '\252' >> $@

clean:
	rm -rf ${BUILD}
