# K05 v1903: original PDP-6 rollback, second FULL commit and typed WAIT

Date: 2026-11-24. **Scoped source-built physical PASS.** The existing
v1902 K05 slot-0 two-owner rollback is now followed by an actual
second native FULL commit and normal original five-child EXIT/WAIT,
parent Z and PDP-6 HALT. Production DAIMOS source is unchanged;
K05-K09 remain open and PURE/mode3 remain default OFF.

## Exact original experiment

The source-built 48K PDP-6 boots compressed secondary TSFS, runs
five original RUN/PIPE children, grows the parent's BRK under real
memory pressure and starts FULL for a PACKED RAW child.
The original native PACKED `mm_free` at `036030` is temporarily
blocked by an external TEST-ONLY pin; at return `036031` it reports
MM_ERR_BUSY=-5 with one RAW source and one complete temporary
PROCESS owner. The original process remains TRANSITION-held with
its private executive stack, FILE state and backing intact.

The controller releases only that test pin. Original slot-0 idle
maintenance executes actual `mm_free(PROCESS)` at `033263`; return
`033264` is MM_OK=0, reducing the MM extent count from 18 to 17.
The native recovery clears TRANSITION at `033313` with the sole RAW
source intact. The saved executive CTX_PC and FDCTL are identical
before and after rollback; the resident parent continues real user
heartbeats and the PCLK advances.

The original scheduler next retries FULL, allocating a new complete
PROCESS target and raising MM occupancy to 18. Its second source
`mm_free(PACKED)` now returns **MM_OK=0** at `036031`, dropping
occupancy to 17 and leaving the sole PROCESS target at
`120000..130000`, pin0. At original service return `033574`,
slot3 is RESIDENT, VMbase `120000`, TRANSITION clear.

Only AFTER exact source/target and process publication is observed
does the simulator set the test-only USER memory gate at `135335`.
The parent exits its heartbeat loop and executes the original five
typed `dsys_wait` calls. Five child outputs `w`, original parent
`Z`, and real PDP-6 HALT PC `000000` occur. Occupancy falls to
8 extents at Z and 6 at HALT, with no residual slot3 PROCESS or
PACKED MM descriptor; the selected ROOT is unchanged.

## Source/build provenance and controller correction

The installed PDP-10 compiler changed after the accepted v1902
build. From unchanged current DAIMOS sources, KCORE DXR IMAGE
becomes `024167` octal (+2 over v1902), BSS `000415`, while
optional PURE IMAGE remains `004615`. Transient KINIT placement
also shifts the installed PURE base from `032435` to `032453`;
real runtime proc_table is `045534`, slot3 u-area `046653`.
These linked and runtime values are cross-checked against actual
original MM, process and source-linked relocation witnesses.
Neither a production KCORE change nor a compiler correctness bug
is claimed from the size delta.

The first exploratory run was **rejected**: SIMH bare `nobreak`
does not clear prior breakpoints, causing a new FULL source CALL
to be mislabeled as its source-free RETURN. This behavior was
verified directly with the simulator. v1903 explicitly uses
`nobreak all` at all ten checkpoint boundaries. A second trace
was stopped before acceptance after identifying the new runtime
proc_table/u-area relocation. Only the corrected third physical
run with genuine native return-PC evidence is accepted.

Production image SHA256:

- KCORE `a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`
- PURE `55be5113d4d15fe5c4a44409dca22afd463c75f05f661b3f8f71451a057c577f`
- Accepted full physical trace `03a3763c12f1da7a8dc6297e8857639d46f64ed63fe5517ec7f2dc16552e3796`

Ten separately corrupted actual physical traces were rejected,
covering initial/target/second-source native return falsification,
wrong MM owner count, premature TRANSITION or user-gate publication,
missing committed APR, changed saved CTX_PC, absent typed WAIT and
missing real HALT. The primary source-linked physical verifier and
all ten negative controls PASS.

Testkit: `daimos-testkit/tests/validation/pure-k05-full-commit-v1903/`.
Permanent documentation: `pdp10-doc/KERNEL/PURE-K05-FULL-COMMIT-V1903.MD`.
Accepted private simulator evidence:
`$TMPDIR/daimos-k05-full-commit-v1903-physical-v3/`.

**Limits:** This closes successful temporary source-pin release,
automatic target rollback, new FULL commit and normal EXIT/WAIT.
It does NOT close permanent external pins, foreign or uncertain MM
owners, terminal failed-FULL teardown, arbitrary PI6 interleavings,
KA10, K05-K09 in full, or optional mode3.
