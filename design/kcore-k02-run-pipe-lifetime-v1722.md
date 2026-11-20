# K02 RUN/PIPE/FILE reference ownership and mixed release refusal v1722

Date: 2026-11-20. Status: native PDP-6 accepted, K02 still OPEN.

## Exact source contract

The current `proc_run_block` constructs a child in `SIDL`. Its FD-map
loop validates each destination before copying the parent's two-word
FILE entry. A malformed or duplicate child destination can therefore
leave speculative words in the unpublished u-area, but those words
are not yet owned references to PIPE. Only after the entire map has
passed does `pipe_add_refs` acquire the inherited PIPE references.
There is no fallible operation between successful acquisition and
the `SRUN`/runqueue publication. Failed-run `proc_slot_discard` must
not close the speculative FDs; doing so could decrement the parent's
original PIPE reference count. This ordering does not require new
reference counters or persistent per-process storage.

`file_close_all_at_v865` processes a specific stable u-area's table,
never replacing the caller's global FILE pointer. Successful closes
clear only the exact descriptor. A failed VFS close or PIPE MM free
must retain the outstanding descriptor, not replay already completed
PIPE decrements or lose the provider ownership. In a fatal process
close, the first actual failed FD and signed provider result occupy
the existing u-area witness word 067, cleared on a later fully
successful pass.

## Native integrated source-linked test

`daimos-testkit/tests/validation/daimos-k02-run-pipe-lifetime-v1722.sh`
links these unchanged production assembly subsections into one native
PDP-6 fixture:

- `proc_run_map_loop` from `proc_run.s`.
- `pipe_add_ref`, `pipe_add_refs`, `pipe_close_ref`, and FIFO unlink
  from `pipe.s`.
- `file_close_entry_v865` and `file_close_all_at_v865` from
  `file_runtime.s`.

The fixture first rejects an FD map with a duplicated child destination
after its first speculative FILE copy and asserts no PIPE reference
acquisition. The valid map inherits two independent copies of the
parent PIPE reader, one writer, and a TSFS mount-8 descriptor with the
same low provider/kind bits but not PIPE ownership. Acquired child refs
produce readers,,writers = `3,,2` including the parent's `1,,1`.

First child FILE-close pass decrements all three child PIPE references
exactly once to `1,,1` but receives an injected VFS_SYNC -5 for its
TSFS FD3. Only FD3 survives and word 067 contains the actual FD3/-5
failure witness. The second pass closes FD3, clears that witness and
does not decrement PIPE again. The parent's next close pass releases
its last reader and attempts to release the final PIPE writer. Test
MM responds MM_ERR_BUSY (-5), preserving the writer FD and `0,,1`
reference counts, with FD1/-5 witness. A final retry accepts exactly
one MM free and clears the witness. The foreign TSFS-like fake PIPE
object must never receive a PIPE reference in any stage.

Two source-mutating negative controls are required to fail with a
meaningful guest result: (1) return immediately from `pipe_add_refs`
without acquiring child refs, and (2) clear FILE descriptors even
when a close reports failure. The production fixture and both mutants
were run; the production result passed, both mutants were rejected.

## Stale encoding test repaired

The old `daimos-run-fdmount-v19` test still expected the obsolete
two-bit mount-extension bitmap in u-area word 067. FILE v417 stores
the complete six-bit vnode mount directly in the first word. The
test now verifies copy of TSFS mount 63 and its file offset into a
new child descriptor, leaving word 067 untouched for finalization
and FILE-close failure witnesses. The original bitmap expectation
failed; the corrected native test passes. The separate
`daimos-pipe-mountref-v19` test also passes (TSFS mount 8 does not
acquire PIPE references).

## Footprint and remaining K02

KCORE source, boot interface, linked text, resident data and runtime
MM allocation are unchanged. Only testkit and K02 open-work tracking
are updated. This source-linked native test is not a full physical
PDP-6 multi-process/interrupt interleaving test and cannot close K02.
## Independent physical and native confirmation (v1722a)

The pre-existing testkit `daimos-fifo-pinned-final-v900.sh` builds a
complete, isolated 256K PDP-6 guest from current KCORE sources, then
physically pins the actual MM owner of a named FIFO at final close.
It rejects the first `mm_free` with genuine MM_ERR_BUSY (-5), restores
the object's FIFO list membership, reopens the same live FIFO and
later unpins and retries. The full guest passed with this observed
counter set:

| Diagnostic | Count |
| --- | ---: |
| Selected FIFO object base (decimal) | 21,534 |
| Final-close attempts | 3 |
| Real `mm_pin` / `mm_unpin` operations | 1 / 1 |
| Real MM_ERR_BUSY refusals | 1 |
| Test errors | 0 |

The guest reached its expected `HALT` without an error marker. SHA-256
of the instrumented guest's KCORE DXR:
`bfbae903741fa2e153f494c8f6aab80477ebd64df93ed5f0de5e69178b30bd19`.
SHA-256 of its SIMH transcript:
`b78eecdc0c55ed670e67c0363466f64cf6e583e82c4b8ac4f2bd597b54a53cf2`.
These are guest artifacts, not changes to production KCORE.

Additional native tests passed against current production source:
`daimos-pipe-typed-close-v1099` verifies separate -5 and -4 refusal
identities, FD retention and named FIFO list ownership with four
negative mutants; `daimos-fifo-open-abort-mm-v1084` checks interrupted
OPEN with pinned MM rollback and eventual close; and
`daimos-pipe-directmount-20261026-v417` confirms that mounted TSFS
vnode identities never acquire PIPE references. v1084 and v1099 needed
only a source-extraction correction excluding unrelated DTFS
administrative abandonment, which was added after their original tests.

This evidence does not exhaust all provider or scheduler interleavings.
K02 remains OPEN and K03 automatic OOM termination remains disabled.
