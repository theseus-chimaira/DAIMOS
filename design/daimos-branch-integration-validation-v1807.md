# DAIMOS RAW PURE integration validation v1807

Date: 2026-11-22. Candidate:
`kcore-raw-pure-integration-v1804`, source tip
`e5110d6f` before this report. Main remains v1803
`495060f6`. Testkit source-matched integration branch
`testkit-pure-integration-v1804`.

## Native source and negative controls

The integrated tree passed the following source-linked PDP-6
native gates on the testkit integration branch:

- v1677 two-owner PI6 fault/READY_ONCE, including unsafe pin mutant
- v1678 MM precommit/rebase refusal
- v1679 repeated persistent MM refusal and bystander isolation
- v1680 terminal pinned-owner retry, including unsafe pin mutant
- v1680 idle PREVM and VM refusal, including early-commit mutant
- v1681 original MM/PURE terminal/finalizer, including four mutants
- v1684 two-source FULL/PURE lifetime, including mount/backing mutants
- v1804 K03 requester-only OOM source controls, historical K04 witness
  verification and combined v1684 native source gates
- v1674 native committed-MM high-water replay controls

The v1680/v1681 tests initially failed to link against the
merged K04 finalizer because their old fixture omitted
`proc_parent_waitable_v1768`, and the old synthetic process
table left INIT absent. They now link the **original source**
parent-eligibility and return tails and create a genuinely
WAIT-eligible RUN INIT in their isolated fixture. v1681
also links the actual PDP-6 PI disable/restore leaf pair.
No production source was modified. Reconciliation and an
opt-in combined `test-pure-integration-native-v1807`
target are committed in daimos-testkit.

## Physical evidence

The v1805 real 256K DSK/D6FS default-OFF guest reached
INIT and login, as previously documented. A v1806 paired
baseline proved the post-unclean-stop DIRTY-root `?RT`
behavior is also present in mainline v1803.

The v1807 real **64K PDP-6 TSFS** guest built from the
integrated source with PURE MRES enabled and bitmap mode3
OFF passed `MEM 64 K`, `DTC OK`, `PURE LOADED` and
`INIT V1` without DSK/DRM attached, remaining alive until
the controlled test timeout. Its source-linked low KCORE
end is `024644` octal. The historical v1665 fixture
requires exact `024255` from the older code snapshot;
an explicit v1807 integration mode instead validates the
linked low-resident bound while retaining all guest checks.
No historical test default was weakened.

The genuine 48K GENERAL/STRICT reclaim/restore pressure
pair is a separate acceptance gate from 64K boot reachability.
The v1673 assembly
child invokes SYS_EXT_SLEEP with `0104600` octal ticks
(**35,200 decimal**, approximately **586.7 seconds**
at 60 Hz) before checking its preserved suffix and
exiting. The historical source comment called this
36,000 ticks/600 seconds, which was inaccurate; the
testkit comment was corrected without changing its
instruction. The default per-checkpoint host deadline
is 900 seconds. A narrower 650-second diagnostic timeout
is **not** an equivalent release gate.

The integrated **48K GENERAL** run produced complete
physical PDP-6 checkpoints A/B/C, with nine RAW PACKED
owners at B and none after WAIT at C. Its measured MM
free words were **26997 -> 2350 -> 26997** and largest
contiguous free words **26997 -> 1197 -> 26997**. The
original native physical oracle PASSED against the
source-matched KCORE map. The integrated source reported
exactly 18173 resident words at each checkpoint.

The historical paired comparator pins 17729 words from
an older source snapshot. The explicit v1807 integration
mode instead pins the observed 18173, with every other
MM, owner, mode and state invariant unchanged. The old
default rejects the new-source witness. All five
negative physical-transcript mutations and the forged
MM summary were rejected by the original oracle under
the integrated profile. The independent STRICT
comparison subsequently completed, as documented below.

## Paired 48K physical acceptance

Both the real GENERAL and STRICT source-built 48K TSFS
guests independently completed original physical CTY A/B/C
checkpoints. The tenth native RUN was refused with a spare
process slot; nine original RAW PACKED owners at B were
removed after nine successful native EXIT/WAIT lifecycles.
At both guests the MM free-word sequence was
**26997 -> 2350 -> 26997** and the largest free extent
sequence was **26997 -> 1197 -> 26997**. Each retained
1,152 words of former PURE prefix gain at pressure (nine
owners times 128 words), with no residual PACKED owner or
pin at C. Both show resident words 18,173, unchanged
throughout their own lifecycles. Physical owner/mode/VM,
arena and source-backed invariants matched across both
profiles at each phase.

The original v1673 runner's final historical comparison
failed on the **expected 17729-vs-18173 source snapshot
resident mismatch**, after both real PDP-6 guests and
their source-linked physical decoders independently
reported PASS. The testkit v1807 comparator explicitly
selects the 18173-word integrated source profile and
leaves the historical 17729-word default untouched.
Its paired verification **PASSed**. Each independently
captured guest transcript then rejected the original
five physical MM/mode/CTY mutants and one forged-summary
mutation, for **12 negative checks total**. A separate
duplicated-guest-transcript mutation was rejected by
the integrated verifier.

Source evidence: `$TMPDIR/daimos-v1673-runtime-physical.QNmaZi`.
The verified pair log is `$TMPDIR/daimos-v1807-paired-verified-fixed.log`.
The two KINIT maps also confirm that STRICT lacks
the disk SWAP MRES and GENERAL retains it. The control
test has no attached DSK/DRM swap media.

This closes **only** the paired 48K raw reclaim/restore
and WAIT test profile. It does not prove physical fault
injection into immutable backing, persistent MM refusal,
continuous MM high-water, RAW 64K pressured reclaim,
bitmap mode3, or the outstanding K05-K08 behavior matrix.

## Promotion boundary

Source-level branch ancestry is consolidated; all named
DAIMOS and testkit heads are reachable from their isolated
integration candidates. Both main branches, all dirty
worktrees, detached archival tags, and `pdp10-doc` remain
unchanged. K01-K04 stay CLOSED.

The +48-word default-OFF permanent-address delta from v1804
is still present; it is not fixed KCORE growth.
RAW PURE K05-K08 lifetime, refusal and scheduler obligations
remain open. K09 mainline promotion and obsolete-branch
retirement require separate acceptance; do not merge
default-ON features or delete protected topic history now.
