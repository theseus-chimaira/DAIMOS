# K05 v1900: exact dual-owner terminal cleanup after failed FULL restore

Date: 2026-11-23. Original PDP-6 native terminal/finalizer
acceptance PASS. K05-K09 remain OPEN; RAW PURE and mode3 remain
disabled by default. Fixed KCORE, process fields, MM descriptors
and resident BSS are unchanged. The optional PURE terminal MRES
contains a necessary source-level ownership fix.

## Original production defect, reproduced on the PDP-6

When original FULL restore cannot complete, the exact v1839/v1855
owner policy intentionally leaves slot3 TRANSITION/VMbase0.
Actual physical v1895, v1896 and v1898 show the authoritative
PACKED RAW suffix `111200..120000` and a second complete
temporary PROCESS target `120000..130000` can both remain
allocated. The original `pdp6_pure_detach_v1609` terminal
MRES leaf, however, would accept only one slot-owned PACKED
extent and rejected ANY PROCESS owner of the dying slot.
Native original PDP-6 original-ASM/VM teardown reproducibly
fails with `__test_exit=000000001001` octal on this exact
two-owner state. It does not free another owner, but the
post-VM FINALIZE+PREVM retry path cannot ever make progress
even after a physically held pin is released. This is a
legitimate missing terminal transition, not a scheduler race.

## Corrected terminal ownership transaction

The v1900 original ASM leaf still checks PACKED storage mode,
TRANSITION, u-area backing identity and exact source RAW length
and verifies no runnable process APR. Under a saved global PI
enable state it now scans the original live MM table bounded
by both `MM_MAX_EXTENTS` (136) and `mm_extent_capacity`. It
accepts exactly one authoritative, unpinned slot-owned PACKED
suffix and optionally one distinct unpinned PROCESS allocation
whose original APR size and physical alignment match the
quarantined process. Any duplicate, extra, pinned, incorrect
size/type/owner or unproven process descriptor causes
`MM_ERR_BUSY`/terminal failure BEFORE any owner is freed.
MODULE and KERNEL_DYNAMIC use independent owner namespaces;
unrecognized types carrying this process slot fail closed.

If an exact temporary PROCESS target is present, the terminal
first invokes the unchanged native `mm_free(PROCESS,slot)`. A
genuine failed target free keeps the authoritative RAW suffix
unchanged, retaining the PREVM finalizer for its existing
bounded backoff. A physically successful target deletion with
a lost error reply also leaves RAW: the next PREVM retry sees
the remaining sole exact PACKED owner and releases it.

Only after the target free reports completion may the terminal
invoke the original native `mm_free(PACKED,slot)` for the RAW
source. If that final native call reports an error, a PI-
excluded post-free scan requires that the original source base
and ALL process-owned PROCESS/PACKED extents are actually gone
before committing storage mode PACKED->RESIDENT. A genuine
refusal or reused/foreign source base stays noncommitted.
Neither ambiguous replies nor a failed `mm_free` can publish
RESIDENT or allow duplicate physical ownership. On successful
publication the original VM-destroy caller clears its original
backing/VM state and the FINALIZE+PREVM reaper advances exactly
as before. No new state field or special fallback is introduced.

The scan and both native frees are serialized by the original
PDP-6 global PI gate, with exactly the previous enable state
restored on all success/refusal exits. The existing private
executive stack and ACs carry all temporary state. This
retains KCORE size unchanged and adds NO BSS words.

## Real source-built memory cost

Source-built actual 48K PDP-6 KCORE DXR remains BYTE-IDENTICAL
to accepted v1855-v1899, SHA256:
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.

Original opt-in PURE MRES DXR SHA256:
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
Corrected opt-in PURE MRES DXR SHA256:
`09bcb71e4cbb172222f6389153c27293353fdaf6d3210eeef4630ed06de934f2`.

Original installed PURE image has `004027` octal words and zero
BSS; current image has `004131` octal words and zero BSS.
Increase is **`000102` octal = 66 decimal installed 36-bit
words**, confined entirely to the removable/optional PURE
MRES. The on-disk DXR increases from 17056 to 17600 bytes;
this represents 66 added executable words and two added
relocation-map words (`000072` -> `000074` octal). Original
linked terminal entry offset remains `003675` octal.

The larger installed PURE moves the source-built runtime
process-table allocation by the same `000102` octal words:
previous original physical `044712`, new source-built runtime
`045014`. A test-only physical oracle must derive this
shift from the original accepted proc-table witness and the
new installed MRES image length, rather than retaining the
old static debugger address. This is a heap allocation shift,
not a change to the fixed KCORE process-table pointer word.

## Original machine and testkit acceptance

`daimos-testkit/tests/validation/pure-k05-terminal-dual-owner-v1900/`
source-links the actual terminal ASM, the original VM destroy
ASM, C swap-detach code and storage-state publication on the
PDP-6 simulator. The sole testkit-only MM stub extends the
historical v1609 native fixture to allow a complete provisional
PROCESS owner and deliberately lost or refused MM replies.
Native cases verify exact two-owner teardown, idempotent
repeat call, held source or target pin refusal with retry,
foreign PROCESS owner conservation, wrong target size, extra
target/invalid type/over-capacity refusal, committed target
free with lost result, committed RAW free with lost result,
and both genuine target/source MM free refusals.

FOUR source-level unsafe terminal assembler variants are
compiled and executed on PDP-6; all are rejected at exact
expected assertions: bypass RAW pin (`001103`), bypass PROCESS
pin (`001203`), bypass exact target length (`001305`), and
publish despite uncommitted RAW free (`001714`) octal.

The stronger v1900 source-extracted native integrated case
uses **actual original MM pin/unpin/free instructions** together
with the actual fixed-KCORE idle FINALIZE+PREVM retry/backoff
service and corrected PURE terminal. With two real MM owners,
the first disposal is deferred by the original source pin;
subsequent original retry after release deletes the complete
target and RAW suffix, reaches `VM_NONE` and publishes a
waitable zombie. Historical v1609/v1680/v1681 native terminal,
pin refusal, original MM/finalizer and executed unsafe tests
also PASS with only test-oracle source-guard-anchor updates.

The separate source-built 48K original simulator controller
**PASSes**: the opt-in PURE package really installs on a
no-AUXSTORE/TSFS boot, re-executes the original native FULL
`mm_free(PACKED)` with an authentic persistent-pin -5 error,
retains all 18 exact MM extents and slot3 TRANSITION/VMbase0,
and observes two independent resident-parent user HBs with
advancing original PCLK `021574` -> `021674` octal. All TEN
original v1895 executed unsafe physical negative transcript
controls were re-run and correctly rejected against the new
PURE hash. The current source-built machine places its proc
table at `045014` rather than old `044712` because the PURE MRES
increased by 66 words. The private source-linked controller
derives that relocation from the actual original table pointer
and linked MRES image length, then checks the real table, slot3
meta `045025=046133600401` (TRANSITION) and VM word
`045026=010000000000` (no published APR). The accepted full
real physical normalized PDP-6 transcript SHA256 is
`91d9cacd28c8f6de77640340f30a431e6e74f23959eabc411fcdcc033aa45a55`.
The historical v1893-v1899 full physical audit matrix also
re-PASSes with all 74 original executed negative controls.

An initial post-link TEST-ONLY root image rebuild replaced the
compressed `/SYSTEM/COMP` fixture with bare TSFS tapes; the
original guest correctly failed `!3`. Restoring the original
test-only compressed root tapes AFTER final link, without
modifying production binaries, resolved the setup error. The
reproducible preflight now checks those exact root tapes to
prevent regression. This proves original KCORE/PURE integration and
quarantine progress, NOT yet a terminal dual-owner fatal
signal/EXIT exercised inside the FULL 48K physical image.

## Remaining gates

K05 still requires whole-guest physical fatal event/EXIT of an
actually quarantined two-owner child, demonstrating persistent
pin refusal, subsequent pin release, MM finalizer backoff,
terminal owner release, parent WAIT and no leaked owner under
real 48K memory pressure. Foreign-owner ambiguity and other
interrupted/full restore paths remain. K06 ownership uncertainty
and K08 scheduling also remain separate open work. RAW PURE and
mode3 remain OFF by default; source changes only in optional
PURE terminal MRES and testkit. Permanent documentation is in
`pdp10-doc/KERNEL/PURE-K05-TERMINAL-DUAL-OWNER-V1900.MD`.
