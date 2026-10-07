LIBC_DIR != pwd
PDP10_PREFIX ?=
BUILD_ROOT ?= ${LIBC_DIR}/../build
LIBC_BUILD = ${BUILD_ROOT}/libc
CC = ${PDP10_PREFIX}/bin/kcc
DAS = ${PDP10_PREFIX}/bin/das
DASFLAGS ?= -F
DARC = ${PDP10_PREFIX}/bin/darc
CFLAGS = -Pgnu99 -O -x=pdp6 -m=gas
CPPFLAGS = -I${LIBC_DIR} -I${LIBC_DIR}/../../system/kernel/core \
	-I${LIBC_DIR}/../../system/kernel/fs -I${LIBC_DIR}/../../system/kernel/proc \
	-I${LIBC_DIR}/../../system/kernel/storage \
	-I${PDP10_PREFIX}/include
OBJS = ${LIBC_BUILD}/memcpy.dobj ${LIBC_BUILD}/memmove.dobj \
	${LIBC_BUILD}/stdlib.dobj ${LIBC_BUILD}/string.dobj ${LIBC_BUILD}/stat_time.dobj \
	${LIBC_BUILD}/process_ctype.dobj ${LIBC_BUILD}/u.dobj ${LIBC_BUILD}/syscall.dobj
LIBC = ${LIBC_BUILD}/libc.a

.PHONY: build install clean check-prefix

build: check-prefix ${LIBC}

check-prefix:
	@test -n "${PDP10_PREFIX}" || { echo "PDP10_PREFIX must be set" >&2; exit 1; }
	@test -x "${CC}" || { echo "missing compiler: ${CC}" >&2; exit 1; }
	@test -x "${DAS}" || { echo "missing assembler: ${DAS}" >&2; exit 1; }
	@test -x "${DARC}" || { echo "missing archiver: ${DARC}" >&2; exit 1; }

${LIBC_BUILD}:
	mkdir -p "$@"

${LIBC_BUILD}/memcpy.dobj: memcpy.s ${LIBC_BUILD}
	"${DAS}" ${DASFLAGS} -C -O "$@" memcpy.s

${LIBC_BUILD}/memmove.dobj: memmove.s ${LIBC_BUILD}
	"${DAS}" ${DASFLAGS} -C -O "$@" memmove.s

${LIBC_BUILD}/syscall.dobj: syscall.s ${LIBC_BUILD}
	"${DAS}" ${DASFLAGS} -C -O "$@" syscall.s

${LIBC_BUILD}/stdlib.dobj: stdlib.c ${CC} ${LIBC_BUILD}
	"${CC}" ${CFLAGS} ${CPPFLAGS} -S stdlib.c -o "${LIBC_BUILD}/stdlib.s"
	"${DAS}" ${DASFLAGS} -C -O "$@" "${LIBC_BUILD}/stdlib.s"

${LIBC_BUILD}/string.dobj: string.c ${CC} ${LIBC_BUILD}
	"${CC}" ${CFLAGS} ${CPPFLAGS} -S string.c -o "${LIBC_BUILD}/string.s"
	"${DAS}" ${DASFLAGS} -C -O "$@" "${LIBC_BUILD}/string.s"

${LIBC_BUILD}/stat_time.dobj: stat_time.c ${CC} ${LIBC_BUILD}
	"${CC}" ${CFLAGS} ${CPPFLAGS} -S stat_time.c -o "${LIBC_BUILD}/stat_time.s"
	"${DAS}" ${DASFLAGS} -C -O "$@" "${LIBC_BUILD}/stat_time.s"

${LIBC_BUILD}/process_ctype.dobj: process_ctype.c ${CC} ${LIBC_BUILD}
	"${CC}" ${CFLAGS} ${CPPFLAGS} -S process_ctype.c -o "${LIBC_BUILD}/process_ctype.s"
	"${DAS}" ${DASFLAGS} -C -O "$@" "${LIBC_BUILD}/process_ctype.s"

${LIBC_BUILD}/u.dobj: u.c ${CC} ${LIBC_BUILD}
	"${CC}" ${CFLAGS} ${CPPFLAGS} -S u.c -o "${LIBC_BUILD}/u.s"
	"${DAS}" ${DASFLAGS} -C -O "$@" "${LIBC_BUILD}/u.s"

${LIBC}: ${OBJS}
	"${DARC}" -o "$@" ${OBJS}

install: build
	mkdir -p "${PDP10_PREFIX}/lib"
	cp "${LIBC}" "${PDP10_PREFIX}/lib/libc.a"

clean:
	rm -rf "${LIBC_BUILD}"
