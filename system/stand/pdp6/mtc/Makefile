PROJECT_ROOT != cd ../../../.. && pwd
BUILD ?= ${PROJECT_ROOT}/build/system/stand/pdp6/mtc
PDP10_PREFIX ?= ${HOME}/cross
DAS ?= ${PDP10_PREFIX}/bin/das
DXRCONVERT ?= ${PDP10_PREFIX}/bin/dxrconvert
STAGE1 = ${PROJECT_ROOT}/system/stand/pdp6/mtc/stage1.s
STAGE1_DXR = ${BUILD}/stage1-mtc-v1.dxr
STAGE1_TAPE = ${BUILD}/stage1-mtc-v1.pt
STAGE1_BASE = 060000

.PHONY: build clean

build: ${STAGE1_TAPE}

${BUILD}:
	mkdir -p $@

${STAGE1_DXR}: ${STAGE1} ${BUILD}
	${DAS} --base-kernel -o $@ $<

${STAGE1_TAPE}: ${STAGE1_DXR}
	${DXRCONVERT} --pdp6-readin -b ${STAGE1_BASE} $< $@

clean:
	rm -rf ${BUILD}
