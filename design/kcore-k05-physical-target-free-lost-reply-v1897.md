# K05 v1897: physically committed target cleanup free, false failed reply

Date: 2026-11-23. Status: **scoped original 48K PDP-6 physical PASS**,
all five original child typed EXIT/WAIT, final parent Z and real
machine HALT PASS. Fixed KCORE and optional PURE MRES are UNCHANGED.
K05-K09 OPEN. RAW PURE and mode3 remain OFF by default.

## What a lost successful rollback target free means

The original FULL transaction retains the authoritative PACKED
RAW suffix until the new PROCESS target has been completely loaded
and verified. If the original `mm_free(PACKED)` refuses while both
original owners still exist and are unpinned, the v1839 exact
owner-set policy safely chooses the separate provisional-target
`fail_target` cleanup. A later error reply from
`mm_free(target,MM_TYPE_PROCESS,slot)` does NOT prove that the
target allocation still exists. Original C deliberately retains
the source, leaves the process TRANSITION and returns `MM_ERR_BUSY`
on any such error; no speculative re-free or premature RESIDENT
publication is attempted. A subsequent retry can safely use the
authoritative source if the completed target deletion committed.

v1896 physically proved a GENUINE target-free refusal with a
permanent target pin: original source pin0 and full target pin1
remained held, original process stayed quarantined, and unrelated
resident parent progress continued. v1897 instead executes the
ORIGINAL same target-free call with pin0, proves the native
`mm_free` really deleted the target with AC1=0 and only THEN
injects a false `-5` AC1 return at the original native return
consumer. This is exactly the opposite physical MM outcome.

## Source-linked original machine and geometry

The original v1855 source-built 48K five-child PDP-6 hardware
guest is reused without modifying original INIT, KCORE, PURE
MRES, child programs or backing media. The original TSFS-root
executable, four compressed secondary-DTC3 children, normal
RUN/BRK/PIPE pressure, writable sentinels and five child
EXIT/WAIT assertions run with the original user Z/HALT.

The actual original KCORE reclaim JRST physically locates the
installed PURE MRES at `032417` octal. Original compiled FULL
DOBJ symbol `.text+000504` and its two distinct `mm_free`
relocations `.text+000674` and `.text+000755` are checked
against the original linked `PUSHJ 17,mm_free` opcode at both
runtime call sites. Original physical PCs (octal):

* bounded FULL restore loader `036202`;
* original authoritative PACKED-source free `035310`, return
  `035311`;
* original provisional PROCESS-target cleanup free `035371`,
  return **`035372`**;
* original FULL service return `033067`.

Original pinned FULL MM count18 has slot3 authoritative PACKED
RAW `111200..120000` pin1 and complete PROCESS target
`120000..130000` pin1. The unchanged original loader runs and
successfully unpins both. A test-only temporary source pin
causes ORIGINAL native `mm_free(PACKED)` to genuinely refuse
with `MM_ERR_BUSY=-5`, leaving both original extents alive. The
test controller then releases ONLY its external source pin,
restoring two unique exact zero-pin original MM owners. Original
v1839 postfailure source owner scan enters the real `fail_target`
at physical `035371`, with BOTH exact original extents pin0 and
all 18 physical descriptors otherwise unchanged.

The original fixed-KCORE `mm_free(PROCESS)` runs completely,
returning **AC1=0** at real runtime PC `035372`. A complete MM
snapshot at this breakpoint confirms target `120000..130000`
is physically GONE, original MM count has dropped **18->17**,
and the sole slot3 MM owner is the exact authoritative PACKED
`111200..120000` pin0; every unrelated MM descriptor, pin,
original selected TSFS root `070101000000` and code image
remain unchanged. No target pin is injected in this experiment.

At this precise postcommit return instruction, the private
test controller deposits ONLY AC1 `777777777773` (=-5), after
verifying the native original result was zero and that the
target was deleted. The original untouched C caller returns
`MM_ERR_BUSY=-5` at FULL service PC `033067`. Original slot3
process meta `044723=046031600401` retains TRANSITION, its
VM word `044724=010000000000` has physical base0, and its
sole PACKED source owner is still pin0 (MM count17). No
invalid runnable PROCESS APR or lost RAW source is observed.

The original user workload later retries the child, completes
all five original PIPE/EXIT/WAIT typed status and writable data
checks, emits parent `Z`, and reaches a real
`HALT instruction, PC: 000000`. Final original MM count is 8;
slot3 PACKED/PROCESS owners are both absent after ordinary
terminal cleanup. Thus conservative reporting of the false
target-free failure is safe and does not prevent eventual
healthy completion in this original physical fixture.

## Physical MM checkpoints (octal)

| Original checkpoint | Original MM extents | PACKED source pin | PROCESS target pin |
|---|---:|---:|---|
| Original pinned FULL | 18 | 1 | 1 |
| Native source free entry | 18 | 0 | 0 |
| Genuine native source free -5 | 18 | 1 | 0 |
| External source pin released | 18 | 0 | 0 |
| Original target cleanup-free call | 18 | 0 | 0 |
| Native target free completes | 17 | 0 | absent |
| Test-only AC1 changed from 0 to -5 | 17 | 0 | absent |
| Original FULL returns -5, TRANSITION held | 17 | 0 | absent |
| Final five child EXIT/WAIT and parent HALT | 8 | absent | absent |

## Independent physical audit and scope

The read-only source-linked physical audit independently verifies
both compiled/linked MM call sites, exact original physical
MM owner/pin/geometry before both frees, actual original
native source-free refusal, true native target-free success and
physical target deletion, the ONE altered postcommit AC1,
original error/nonpublication process result, owner/root
conservation, five original typed child checks and final HALT.
**TWELVE separately executed forged physical transcript mutants**
are rejected at intended owner, pin, result, metadata, terminal
MM or HALT checks. The test-only negative oracle was corrected
to recognize that the original MM count prints TWICE in a
single original simulator snapshot; only the test-only oracle
changed, not the original physical simulator data.

Accepted normalized original physical PDP-6 trace SHA256:
`6aeb9da4b13d471ca300b2091b4b663be867f24c3e54850579287f140fa6d11e`.
Unchanged fixed KCORE DXR SHA256:
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
Unchanged optional PURE MRES DXR SHA256:
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
Private original simulator evidence:
`$TMPDIR/daimos-k05-target-free-lost-reply-v1897-real/`.
Executable physical testkit and audit:
`tests/validation/pure-k05-target-free-lost-reply-v1897/`.
Permanent kernel reference:
`pdp10-doc/KERNEL/PURE-K05-TARGET-FREE-LOST-REPLY-V1897.MD`.

This settles the specific **completed target rollback free with
a falsely failed return** and successful subsequent original
retry/EXIT/WAIT/HALT. It does NOT prove foreign physical ownership,
permanently failed unpins, target-free lost reply with intervening
foreign owner, eventual cleanup of permanently held process
extents, arbitrary PI6 timing, 64K/KA10 or K05-K09 release.
RAW PURE/mode3 remain OFF. Fixed KCORE/PURE RAM growth ZERO.

**Physical follow-up v1898 (2026-11-23):** The original compiled
FULL native target `mm_unpin` calls at `035271` and `035362` were
physically executed against a test-only zero target pin. BOTH
original calls genuinely returned -4, causing safe TRANSITION/
VM0 quarantine, source pin1 and target pin0 preserved; two new
unrelated resident-parent HBs and original PCLK progression PASS.
The injected change was inside the normally PI-atomic commit
window, so this proves defensive error handling, not a natural
interrupt race. See
`design/kcore-k05-physical-target-unpin-refusal-v1898.md`.
