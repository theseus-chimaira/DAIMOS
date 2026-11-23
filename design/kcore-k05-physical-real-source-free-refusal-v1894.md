# K05 v1894: original real PDP-6 MM source-free refusal and safe rollback

Date: 2026-11-23. Scoped original 48K full-system physical **PASS**.
No production source changes. K05-K09 OPEN; RAW PURE/mode3 OFF.

## Why v1894 is different from v1893

v1839 fixed the dangerous assumption that a failed `mm_free(PACKED)`
return proves the source still exists. The original code now scans
the exact zero-pin MM owner set under PI exclusion to distinguish:

1. Sole full PROCESS owner: source was actually freed despite its
   error reply; publish the complete successor, never roll it back.
2. Exact PACKED source and complete PROCESS target, both unpinned:
   source free really refused; roll back ONLY the temporary target.
3. Any other owner/size/type/pin/duplicate condition: quarantine
   without speculative release.

v1893 was the first full-system PDP-6 physical exercise of case 1:
the original `mm_free(PACKED)` physically succeeded, source vanished,
then only AC1 was changed to -5, and the correct successor published.
v1894 physically exercises **case 2 with an ORIGINAL native MM -5**,
not an error reply spoof. It uses a genuinely pinned source for
one original fixed-KCORE `mm_free` call and then releases that
transient external pin before original C checks exact owner set.

## Exact original source-linked breakpoint and hardware state

The accepted source-built v1855 original PDP-6 48K five-child
ROOT/secondary-DTC3 PIPE/BRK/RAW PURE fixture is reused WITHOUT
editing user, KCORE, removable PURE MRES, source tapes or scheduler.
The installed PURE base is physically derived from original KCORE
reclaim JRST. The original compiled DOBJ FULL symbol and both
`mm_free` relocations are checked against the real linked PURE
instructions. Unique physical addresses (octal):

* bounded FULL entry `036202`;
* original PACKED-source `PUSHJ 17,mm_free` `035310`;
* genuine original `mm_free` return consumer `035311`;
* original FULL service return `033067`;
* original slot3 PACKED descriptor metadata `024703`.

At the original FULL loader checkpoint, the authoritative MM table
has 18 entries, including PACKED RAW `111200..120000` pin1 and
the separate full PROCESS `120000..130000` pin1. The actual FULL
loader succeeds, copies the RAW suffix, performs the v1855
one-pin owner-pair check and unpins both original MM extents.

The simulator now stops on the original SOURCE `mm_free` call
BEFORE executing it. A real physical snapshot verifies source
`111200..120000` pin0, target `120000..130000` pin0, exact
unchanged unrelated MM records, selected ROOT unchanged, and
original source metadata `024703: 000004000003`. Only a private
SIMH controller deposits `024703: 000014000003` to model a
single external physical pin. This is the **sole temporary owner
injection**; the original code executes the actual unmodified
fixed-KCORE allocator. Its exact source-free check refuses the
pinned PACKED descriptor and returns genuine AC1
`777777777773` = `MM_ERR_BUSY=-5` at `035311`, with BOTH extents
still present, source pin1 and target pin0, original MM count18.

To prove case 2 rather than v1890's uncertain extra-pin quarantine,
the controller immediately releases only the temporary external
source pin with a second private deposit to restore original source
metadata `000004000003`. The actual physical MM snapshot
independently confirms **BOTH exact owners now unpinned**,
MM count18 and unchanged unrelated extents/root. Original v1839
reconciliation then counts both authoritative zero-pin owners,
chooses checked target rollback, frees the temporary PROCESS
`120000..130000` and preserves source PACKED `111200..120000`
pin0. The original FULL service at physical `033067` returns
`MM_ERR_BUSY=-5`, original slot3 VM base zero, original MM count17,
no invalid RESIDENT APR publication and no loss of the RAW suffix.

The original guest retries after this one-time refusal, completes
all five independent child PIPE reads/typed EXIT/WAIT checks with
writable-data sentinels, prints parent `Z` and physically HALTs at
PC `000000`. Its last original MM snapshot contains 8 extents,
none of them remaining slot3 PACKED/PROCESS. At every checkpoint
the selected TSFS root is `070101000000` octal and no unrelated
original MM owner changes. This acceptance proves both safe
refusal-state cleanup AND normal eventual child lifecycle.

## Actual MM evidence (octal)

| Checkpoint | MM extents | PACKED source | PROCESS target |
|---|---:|---|---|
| Original pinned FULL | 18 | `111200..120000` pin1 | `120000..130000` pin1 |
| Original source-free call | 18 | same pin0 | same pin0 |
| Genuine original `mm_free` -5 | 18 | same pin1 | same pin0 |
| Temporary pin released | 18 | same pin0 | same pin0 |
| Original FULL returns -5 | 17 | same pin0 | absent |
| Original parent `Z`/HALT | 8 | absent (child reaped) | absent |

The original runtime and original no-fault image were NOT changed.
The two temporary physical MM metadata deposits are test-only.
This is a transient real MM refusal, NOT a permanent refusal.

## Audit, provenance, regressions

Independent physical oracle checks the actual three original PCs,
the original KCORE/MRES hashes, source/target geometry, MM owner
counts, exact pin metadata before/during/after native `mm_free`,
genuine `-5` AC1 before caller operation, exact two unpinned owners,
target rollback, no source loss, original VM/process-table pointer,
unchanged unrelated owners/TSFS root and terminal state. Seven
SEPARATELY EXECUTED unsafe physical transcript mutants are
rejected at intended checks: original source pre-pin, injection
missing, `mm_free` false success, incorrect refused-owner pin,
external pin not released, incorrect two-unpinned owner set, and
false success at original rollback return.

Source-built fixed KCORE SHA256 (byte-identical v1855-v1893):
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
Original opt-in PURE MRES SHA256 (byte-identical v1855-v1893):
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
Actual original normalized physical PDP-6 trace SHA256:
`4cdba1d83d04a9d92a50a7f13965f389419175cec875f28f58e94f1ebdf97071`.

Actual real-machine evidence:
`$TMPDIR/daimos-k05-real-free-refusal-v1894-real/`.
Testkit only:
`tests/validation/pure-k05-real-free-refusal-v1894/`.
Real original physical run and independent audit targets:
`make test-pure-k05-real-free-refusal-physical-v1894` and
`make test-pure-k05-real-free-refusal-audit-v1894`.

## Remaining gates

This closes ONLY a genuinely executed physical `mm_free(PACKED)`
source refusal for an externally pinned source that is unpinned
BEFORE authoritative two-owner reconciliation. A persistently
held extra source pin must instead remain quarantined. Other
pending cases include permanent target rollback refusal, unknown
allocator result, foreign owner, permanently failed unpin,
terminal cleanup in unresolved TRANSITION, additional PI6 orders,
64K/KA10 and K05-K09 release acceptance. RAW PURE/mode3 remain
disabled by default. Fixed KCORE and opt-in PURE MRES RAM growth
for this milestone: **ZERO words**.
