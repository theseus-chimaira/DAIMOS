# Proposal B v407: provider range and six-bit metadata gate

Status: design-only; no kernel code or ABI changed.

## Verified current provider/local-kind ranges

- MonitorFS device provider 2, process provider 3. Device kinds span
  0..6; process kinds are 1..3 (`fs/monitorfs.h`).
- MEMFS provider 4, local kind 1 (`fs/memfs.h`).
- DTFS provider 5, kinds 1..2 (`fs/dtfs.h`).
- D6FS provider 6, kind 1 (`fs/d6fs_provider.h`).
- TSFS provider 7, kinds 1..2 (`fs/tsfs.h`); PIPE shares provider 7,
  with stream kind 1 and mount 0 (`ipc/pipe.h` and PIPE assembly).

The current source-defined providers/kinds fit three bits each, but the
generic `vfs.h` format admits SIX bits each, and `vfs.s` validates
against that generic 0..63 range. A FILE-specific narrower contract
requires consistent enforcement at every vnode publication boundary.
Provider/local-kind compatibility cannot be inferred for future modules.

## Exact bit masks and limitations

With provider 0..7, local kind 0..7, mount 0..31, index 0..0777777:

  vnode mask    073707777777  (36-bit octal)
  metadata mask 704070000000  (36-bit octal)

Available metadata positions: 21,22,23,29,33,34,35 (seven bits).
With mount 0..63, vnode mask 077707777777 and metadata mask
700070000000, leaving positions 21,22,23,33,34,35 (six bits).

Correction: the initial v406 write-up accidentally printed a
thirteen-digit metadata-mask literal. This review corrected v406's
literal to the verified 12-digit 704070000000. No production mask was
modified.

The historical nine metadata bits cannot fit in either proposed
restricted layout without a coordinated semantics/representation change.
Dropping the four-bit family token would leave five bits, but the
replacement FLOCK contract and inheritance rules have not been agreed.
There is NO accepted six-bit production encoding yet.

## Proposed next evaluation

1. Define per-process FLOCK behavior for same-vnode separate OPEN,
   DUP/DUP2, RUN, CLOSE and last-reference teardown. Explicitly state
   that the current system does not enforce interprocess exclusion.
2. Evaluate a six-bit metadata layout holding access, directory/append,
   and regular/lock state, with one spare bit. Prove all reachable modes,
   closed-slot detection, and lossless vnode reconstruction; this is a
   candidate only, not approval to drop family ownership.
3. Audit all fields of generic `vfs_mount` and external provider
   registration before imposing the three-bit FILE restrictions. Verify
   mount 26 and 63 behavior with distinct identical local vnode IDs.
4. Once approved, compare full linked image and physical RAM before
   accepting the change. Tests exclusively in daimos-testkit.

Reference v388 full-link baseline: KCORE 9109; KCORE+MRES 15657;
PERMANENT_LAST 15704. No new link or runtime acceptance claimed.
