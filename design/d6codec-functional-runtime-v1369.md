# v1369: grouped executable, PURE and TSFS functional runtime tests

Date: 2026-11-08. Status: tested with native PDP-6 runtime code and
controlled filesystem/VM providers, NOT a complete grouped cold boot.
Production D6LZ36 and installed media remain unchanged.

## Prioritize correctness before further size optimization

The v1368 full-KCORE link quantified the additional grouped decoder
RAM but did not exercise actual executable, swap PURE or TSFS consumer
paths. v1369 closes those isolated native functional integration gaps
without adding more production code or optimizing instruction size.

## Grouped DXR2 test producer

`daimos-testkit/tests/validation/d6codec-runtime-exec-v1369/`
contains `grouped_dxr_v1369.py`, converting a real linked DXR1/DXR2
to a grouped DXR2 **test image**. It checks the original 36-bit
container, DXR magic, entry, image size, BSS flags, TX2 text metadata
and exact relocation bitmap length. Grouped encoding and independent
complete decoding must agree on every original word. Output preserves
original entry, other flags, TX2 metadata and relocation words,
setting only the existing `DXR_F_COMPRESSED` flag before publication
under a versioned filename. It refuses existing and legacy-named
targets, recompression and malformed headers without partial output.

**The existing DXR compressed flag does not identify the codec.**
Therefore these grouped DXRs are not safe to mix with production
D6LZ36 executable files; an atomic image and runtime format change
is required for deployment.

## Actual native EXEC and PURE integration

Native PDP-6 tests link the **unchanged** `exec_load_process` and
an instruction-for-instruction extraction of `proc_swap_reload_pure`
against the staged v1367 grouped VFS decoder. Deterministic VFS,
VM and backing vnode stubs supply real grouped executable files,
allow error injection and verify ownership/cleanup.

Two real linked binaries pass:

- A C89-compiled two-instruction marker program, decompressed by
  actual EXEC, then executed at its loaded VM address. It returns
  octal `012345`.
- A 390-word linked tape MRES DXR, compressed into 379 grouped
  words, exercising multiple short seven-word VFS refills and
  complete word-by-word output verification.

Both pass ordinary and PURE-flagged EXEC, actual PURE reload after
evicting text, malformed grouped stream rejection and provider EOF
rollback. Failed EXEC destroys the staged VM and does not attach
swap backing or publish a process entry. Corrupt PURE immutable
backing is rejected. The PURE vnode unpacker itself is stubbed;
its mount/lifetime validation remains a separate integration gate.

## Actual native TSFS read/copy/free integration

A 256-word (octal `0400`) TSFS restart with 32-word repeating data
compresses to **35 grouped words**, within the existing single
DECtape-block restart limit of octal `0176` words. The unchanged
`tsfs_read_words` implementation, linked to the grouped buffer
adapter, returns and checks 20 logical words at offset 8. It frees
the transient 256-word allocation. A corrupted control word yields
the TSFS negative error return and still frees the allocation,
without copying the invalid decompressed result to the caller.

## Acceptance and remaining work

Execute: `DAIMOS_REPO=/path/to/DAIMOS sh
tests/validation/d6codec-runtime-exec-v1369/run-v1369.sh` from the
testkit. This is all isolated under `$TMPDIR/d6codec-runtime-v1369`.

The next critical task is building the **entire** grouped KCORE,
KINIT, MRES packages, root filesystem and user programs as one
isolated coherent boot set, then booting past KINIT to INIT/LOGIN
and executing a grouped user program. Real VFS provider nesting,
FS/device failures, kernel stack high-water and swap ownership also
need acceptance. The v1368 +216/+256-word KCORE cost is unchanged.
