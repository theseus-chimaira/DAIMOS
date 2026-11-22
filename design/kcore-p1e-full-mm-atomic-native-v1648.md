# P1-E v1648: native FULL source-free and destination-pin rollback

Date: 2026-11-17. **Source-linked native PDP-6 PASS.** This is an
additional checked MM transaction test for the opt-in PURE service;
it is not a substitute for a real physical VFS/PI6 FULL race.
Production v1647 KCORE/MM/PURE sources are unchanged.

## Verified original transaction and missing native fault cases

The v1391 FULL restore owns the original single unpinned RAW
`MM_TYPE_PACKED_PROCESS` suffix, independently allocates and pins
an entire APR-sized `MM_TYPE_PROCESS` target, and invokes a bounded
loader. The original suffix overlays the loaded image and remains
authoritative until its checked unpin and release succeed. Only
then may the process publish its resident VM base and storage mode.

The preexisting v1395/v1397 source-linked regression already covers
FULL target unpin/free refusal, source unpin refusal, post-commit
target unpin refusal, and DROP rebase/unpin faults. v1648 adds three
different missing cases, using wrappers only at the compiled
production v1391 MM call boundary:

| Case | Injected return | Native expected behavior |
| --- | --- | --- |
| 7 | Source PACKED `mm_free` returns `MM_ERR_BUSY` | Preserve unpinned source; free temporary full target; no publication |
| 8 | Source PACKED `mm_free` returns `MM_ERR_INVAL` | Same fail-closed rollback |
| 9 | New FULL destination `mm_pin` returns `MM_ERR_BUSY` | Free new target; unpin untouched source; no publication |

For all three the actual compiled v1391 transaction returns the
injected error. Its exact one authoritative PACKED descriptor,
base/size/type/owner/pin0, full original RAW contents, old
executable backing and `TRANSITION` flag remain intact. The
temporary PROCESS descriptor is absent and the runnable APR
base remains zero. Removing the injected fault and retrying
through the *same* compiled transaction yields exactly one
unpinned PROCESS destination with original RAW suffix, correct
immutable text, published APR, RESIDENT storage, cleared backing
and cleared TRANSITION.

The complete unmodified v1395/v1397 native matrix runs first,
as does the separate real v1611/v1612 service regression.

## Negative acceptance controls

Two intentionally invalid test-only copies of the real FULL C
transaction were separately compiled and EXECUTED on native
PDP-6 `p10run`, not merely rejected at assembly time:

* Omit RAW suffix overlay: the new full-data oracle fails with
  executed exit `0706` octal.
* Omit target cleanup only after a source-free `MM_ERR_INVAL`,
  preserving all old v1395 tests: the new descriptor-owner oracle
  fails with executed exit `01004` octal.

Both invalid binaries are rejected. No mutated code is installed
or considered a production fix. Compiling the test source leaves
the historical KCC long-identifier/unused-static notes unchanged;
native link and simulator exit result are PASS.

The versioned regression and detailed run instructions are in
`daimos-testkit/tests/validation/pure-full-atomic-v1648/`, commit
`500d023` on `test-kcore-p1e-full-mm-atomic-v1648-20261117`.

Evidence archive, including exact source-derived compiled assembler,
the unmodified positive exit report, both executed negative reports,
test-source mutations, and SHA256 manifest:
`$TMPDIR/daimos-p1e-v1648-acceptance.bWDUeM.tar.gz`.
SHA256:
`a5ae4c03da30357016b5997122bbbc8a11ec08cccea4c884cc5b71c57162ae8d`.

## Decision / limitations

**Accepted only for tested MM failure paths.** The v1391
transaction is source-linked and checked, but arbitrary MM
corruption, permanently uncertain pins/ownership and simultaneous
user/kernel activity during a blocked FULL provider read remain
unaccepted. Earlier v1638 physical pinned RAW+APR FULL, v1643
real secondary-media read loss, v1645 PI6 owner lifetime and v1647
real DTVOL2 quiescence are independent accepted physical gates.

Next: establish simultaneous pinned RAW and PROCESS target while
a second process requests DTFS source mutation/unmount during an
actual provider operation. No fabricated scheduler switch on
slot 0 or disabling of source backing guards is permitted. Then
measure whole physical transient peak on 32K/48K/64K. KA10 is a
separate VM backend, never a PDP-6 APR alias. PURE stays opt-in;
strict 32K KCORE optimization HOLD, VERSION 0.5.
