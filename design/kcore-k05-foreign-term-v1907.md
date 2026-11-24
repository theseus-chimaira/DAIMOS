# K05 v1907: terminal event while foreign FULL target is escrowed

Date: 2026-11-24. **Original 48K PDP-6 physical PASS.**

The preexisting `proc_event_apply` rejects a TRANSITION-held
target before FILE close, fatal-event coalescing or FINALIZE.
v1901 physically proved TERM refusal with an original saved
executive continuation; v1906 physically proved the exact
foreign-claimed FULL target escrow. The remaining concern was
their interaction, especially the new target base in u[067].

One source-built five-child parent makes ONE genuine user
`SYS_PROCCTL_EVENT_PID` TERM request for slot3 after an
original native PACKED source-free -5 and test-only retagging
of the complete PROCESS target owner3 -> owner2. The request
is enabled only through a separate parent USER APR gate at
`135361` following the first checked foreign-owner HB.
The original parent prints `TR` (TERM refused).

The physical snapshot at TERM proves MM18 with the exact
authoritative RAW and foreign-claimed PROCESS owners, slot3
TRANSITION/VMbase0, unchanged original private CTX_PC/FDCTL,
escrow `040000120000` in its already-existing u[067], ROOT,
and still-closed parent WAIT exit gate `135357`. Two further
parent HBs remain safe. After only explicit test repair of the
corrupt MM owner field, v1906 native rollback, next FULL,
five original typed WAITs and real HALT PC0 pass, MM6 final.
The entire v1906 physical verifier re-PASSes, plus four
new targeted negative mutants; no unnecessary 16-case rerun.

Fixed KCORE and optional PURE MRES are byte-identical to v1906:
KCORE `a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`,
PURE `09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.
Accepted physical trace SHA256:
`da5ae4d1920279f78965080297985b5e698f1ae6ea3390b573a5edb34b26c7dc`.
No production source, process-table, BSS, fixed RAM, optional
PURE image size or policy changed.

Testkit: `daimos-testkit/tests/validation/pure-k05-foreign-term-v1907/`.
Permanent documentation: `pdp10-doc/KERNEL/PURE-K05-FOREIGN-TERM-V1907.MD`.
Accepted trace: `$TMPDIR/daimos-k05-foreign-term-v1907-physical-v1/`.

**Remaining K05:** truly terminal failed-FULL teardown of
an indefinitely foreign/pinned target is unsafe until the
original private executive continuation and MM ownership can
be reconciled. Do not bypass TRANSITION or forge FINALIZE.
Uncertain allocator metadata and other PI6 orderings remain
open. K06-K09 separate; RAW PURE/mode3 default OFF.
