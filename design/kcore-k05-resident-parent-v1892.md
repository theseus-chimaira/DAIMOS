# K05 v1892: original PDP-6 resident parent progresses after FULL quarantine

Date: 2026-11-23. **Scoped full-system 48K PDP-6 physical PASS.**
K05-K09 OPEN. RAW PURE and mode3 OFF by default.

## Previously open scheduling boundary

The original v1855 production correction rejects any FULL commit
whose authoritative PACKED RAW source and separate complete PROCESS
destination are not **exactly** singly pinned and correctly owned.
Physical v1890 injected a second target MM pin at the real original
FULL loader and observed `MM_ERR_BUSY=-5`, exact two-owner retention
and later real PCLK PI6. Physical v1891 additionally proved that
slot3 remains TRANSITION-held with no published VM base, while
slots4..6 keep isolated nontransition PACKED owner descriptors.

The original 48K MM arena after this artificial persistent pin2
contains no aligned free `010000`-word destination for another
PACKED child FULL restore. A disposable attempt waiting for another
child `w` could not establish bystander user progress. A real
already-resident independent process is needed to distinguish a
quarantined child from globally stalled user execution.

## No production source change or additional resident words

v1892 rebuilds the same original 48K five-child RUN/PIPE/BRK
pressure guest with **only a testkit user-INIT fixture change**.
All original child admissions, failed sixth RUN, immutable compressed
secondary-DTC3 backing, BRK growth, memory-accounting assertions,
PIPE and writable-data sentinel checks remain byte-for-byte source
identical before the parent's PIPE write-end close. After the
original `L` pressure marker and `dsys_close(wr)`, the resident
parent (slot1) executes a test-only loop: `dsys_sleep(0100U)`,
validate its two original mutable data sentinels, emit `H` and `B`
using original user syscalls, and repeat. Unlike the previously
blocked parent WAIT loop, this autonomous parent can demonstrate
user-level progress while its child remains permanently quarantined.

No new syscall, kernel scheduler path, process field, MM registry,
driver, device backing format or userland library is required.
The testkit physical boot-builder's explicit
`PURE_PREPARE_ONLY_V1892=1` option only skips its normal guest HALT
check for a deliberately nonterminating INIT. All original v1614
diagnostic behavior with that option OFF is unchanged.

The **actual freshly source-built** permanent KCORE and removable
PURE MRES DXRs are BYTE-IDENTICAL to v1855/v1890/v1891:

* KCORE SHA256
  `33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
* Optional PURE MRES SHA256
  `a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
* The new TEST-ONLY parent INIT DXR SHA256
  `620fcfaddedd5a4abea8b35e11a99e371fa624420ce479a4e4de69fb92c24e27`.

The test-only user INIT linked text is `001222` octal words versus
`001246` in the original blocked-WAIT fixture (20 decimal fewer),
with both BSS values `004001` octal. The original full parent
physical APR is still `010000` words; this must NOT be reported as
production kernel RAM saving. KCORE/PURE memory growth is ZERO.

## Exact original physical fault and independently resident user

The independent v1892 source-linked physical KCORE reclaim-JRST
probe again discovers the installed PURE base `032417` octal and
original bounded FULL loader `036202`, service return `033067`.
At actual pinned slot3 FULL, the original PACKED RAW suffix
`111200..120000` has pin1 and separate full PROCESS target
`120000..130000` has pin1. Only the private SIMH test controller
deposits the exact target MM word at physical `024705`, from
`000011000003` (pin1) to `000021000003` (pin2); the controller
reads both real physical words. DTC3 never detaches. Original
FULL returns `777777777773` = `MM_ERR_BUSY (-5)` at its real
service return. A real permanent KCORE PI6 at `017205` confirms
retained pin1/pin2, all other extents and selected TSFS ROOT.
The real original process-table pointer is `044712`, and slot3
remains TRANSITION-held with physical VM base zero.

**Only after** these original fault/quarantine/PCLK snapshots does
the physical controller clear the PCLK breakpoint and arm a live
console `expect "HB"`. The first NEW user-written `HB` stops the
actual simulator; the controller independently records the original
MM and process table. It resets the same console expectation, then
requires a SECOND newly emitted `HB` and matching snapshots.
Earlier heartbeats before the FULL error are excluded by chronology.

Both actual postfault user output checkpoints PASS:

| Octal original hardware field | First HB | Second HB |
|---|---|---|
| `proc_current_slot` | 1 | 1 |
| Parent slot1 `PROC_SRUN` | 2 | 2 |
| Parent VM base | `130000` | `130000` |
| Original parent MM PROCESS | `130000..140000`, pin0 | unchanged |
| Slot3 TRANSITION / VM base | 1 / 0 | unchanged |
| Slot3 PACKED RAW source pin | 1 | 1 |
| Slot3 temporary PROCESS pin | 2 | 2 |
| `proc_timer_clock` | `021610` | `021710` |

The original physical clock advances by exactly `0100` octal ticks
between new user output events. At both checkpoints the source,
target, unrelated MM extents and selected TSFS ROOT remain held.
This proves an actually **scheduled, resident, autonomous user
process** continues executing ordinary syscalls after another
process's permanently uncertain FULL has been rejected. This is
not merely a scheduler state flag or a PCLK tick.

## Negative controls and evidence provenance

The initial physical v1892 attempt did not prove user progress:
the controller inadvertently left its previous PCLK breakpoint
armed and stopped twice at PCLK instead of actual parent output.
The unchanged guest's source-linked fault/MM test passed but the
heartbeat oracle correctly rejected the witness. The controller
was corrected by adding `nobreak` **before** the two live `HB`
expectations; no production code or test guest was changed.

The second complete source-built physical run and independent
read-only audit PASS. SIX separately executed negative transcripts
are rejected at their exact source-linked checks: a missing second
`HB`, a no-longer-resident parent, an incorrect active user slot,
prematurely cleared slot3 TRANSITION, lost authoritative RAW pin,
and no original PCLK advance between parent HBs. Each mutant's
failed verifier leaves the genuine original trace restored.

Original accepted physical v1892 normalized transcript SHA256:
`7fbc1e9fa203c2c146861d0f951e222f22721f221e3f095c300b23d96ca72f86`.
Actual original simulator evidence and snapshots:
`$TMPDIR/daimos-k05-resident-peer-v1892-physical-v2/`.
Source-built user/boot media:
`$TMPDIR/pure-brk-pressure-v1614.UQzCQO/`.
Original installed-KCORE-JRST source probe:
`$TMPDIR/daimos-k05-resident-peer-probe-v1892/`.
Reproduction, exact source checks, full auditor and versioned tests:
`daimos-testkit/tests/validation/pure-k05-resident-peer-v1892/`.

## What this does not close

The unrelated PACKED secondary children are not proven to
restore, emit new `w`, EXIT or complete parent WAIT/HALT after the
pin2 quarantine. The independent physical user witness is the
**resident parent** whose WAIT was replaced in the test-only guest;
the production user INIT and healthy all-five WAIT/HALT baseline
remain unchanged. Persistent source/target unpin, free and allocator
refusal/uncertainty paths, terminal cleanup, arbitrary PI6 timing,
64K/KA10 and K05-K09 release remain OPEN. Keep RAW PURE/mode3 OFF.
