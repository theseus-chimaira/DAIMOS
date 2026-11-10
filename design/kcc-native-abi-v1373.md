# v1373: fix GCC-ABI nested call arguments in KCC's PDP-6 generator

Date: 2026-11-08. Status: **corrected native single-function KCC
codegen and PDP-6 nested-call ABI execution PASS**. The broader
native self-hosting compiler and the grouped-codec production cutover
remain staged, with separate KIR/global and `-N` defects open.

## Investigation

The v1372 native KCC compiler unexpectedly emitted a blank mnemonic
and `FMPL 017,0` for a valid C89 `return 012345` expression, while
the host compiler emitted `MOVEI 1,012345` and `POPJ 17,`. The first
reproducer compiled successfully and returned exit status zero; a
mere successful compiler invocation did not detect the bug.

The previously installed native KGEN emitted obsolete `KPCODE1`.
Rebuilding all five current-source native phases without installing
them produces `KPCODE2`, but still emitted wrong pseudocode opcodes:
`P_CVALUE` (1), `P_DFMP` (15) and `P_POPJ` (61). The KIR1 readback
contains the correct `N_ICONST` value 5349 decimal (012345 octal).
Instrumented native KGEN showed wrong `op` values on entry to
`CCCODE:NEWCODE`, before the serializer and KOPT could change them.

The culprit was the **host KCC used to cross-compile native KGEN**.
For `code1(int op, VREG *vr, INT s)` the generated assembler called
`vrtoreal(vr)` to calculate argument two of `codr1(op, ..., s)` but
failed to preserve the original `op` passed in volatile AC1. The
third incoming argument could also be destroyed. This is an ABI
code-generation defect, not a bug in `code1()` or in the codec.

## Source correction

KCC `ccgen2.c:gccabi_direct_reg_args()` now checks whether any
argument expression has side effects. If so, it evaluates all
`Register_Id` incoming register arguments **before** evaluating
subsequent arguments and holds them in tracked VREGs. Nested call
spilling and `vrtoreal()` restore those original values to the
correct outgoing ABI register slots. Calls without side effects
retain their direct register fast path. The fix requires no new
permanent register table, no kernel ABI change and no KCORE growth.

## Reproducible full native PDP-6 regression

`daimos-testkit/tests/validation/kcc-native-refresh-v1373/run-v1373.sh`
first builds the patched **host** KCC and verifies its generated
`code1` assembly saves AC1 across `vrtoreal`. It cross-compiles
all five native KCC phases with that exact host compiler, rejecting
malformed DXRs. The test stages only the five new DXRs in a separate
copy of the normal **237-file** grouped D6FS image. No production
source, Makefile, executable or media is overwritten.

The full grouped KCORE plus all 27 compatible MRES packages boots
the new D6FS root under native PDP-6 SIMH. Nine guest probes PASS:
native driver compilation of the minimal C89 return, exact 36-bit
host/native assembler equality, standalone KCPP, KPARSE, KGEN
KPCODE2 header and KOPT operation, exact KGEN/KOPT output equality,
and executing a separately compiled nested-call C89 program that
checks the values of all three register arguments and exits zero.

The complete test's default command is:

    DAIMOS_REPO=/path/to/staged/DAIMOS \
    KCC_REPO=/path/to/staged/kcc \
      sh tests/validation/kcc-native-refresh-v1373/run-v1373.sh

This tests a **minimal single-function native compiler** and a
host-built nested-call executable. It does not yet prove that
native KCC compiles every program or that native DAS can consume
native-generated packed ASCII7 assembler output.

## Remaining blocking compiler defects

1. Native KCC fails on the larger `abi-nested-call-v1373.c` source,
   which has several functions and initialized globals. Native KGEN
   reports `gaddress: bad Sclass 0` on a global identifier during
   KIR1 symbol reconstruction. The separate testkit
   `probes-open-kir-v1373.txt` records this failure; it is *not*
   included among the positive nine checks.
2. Calling standalone native KGEN with `-N` on a correctly generated
   KIR file crashes via a UUO and returns status 126. The normal
   optimized KGEN/KOPT pipeline passes; unoptimized native KGEN is
   not accepted.
3. Native compiler -> ordinary uppercase SIXBIT/S6REC assembly ->
   native DAS/DLINK -> executable RUN remains to be accepted. The
   previously verified v1372 C-SIX converter is a reversible C
   **source** representation, not an assembler-text parser.

These remain high-priority correctness tasks. No production KCC,
resident DAS, grouped KCORE, root media or D6LZ36 format was
installed or changed as part of this stage.
