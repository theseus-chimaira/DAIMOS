# K05 v1847: physical fixed-KCORE PI6 after failed FULL source read

Date: 2026-11-23. Status: scoped physical PDP-6 PASS.
K05 remains OPEN and PURE is disabled by default.

## Source-linked setup

The testkit reuses exactly the accepted v1845 five-child 48K PDP-6
TSFS-root, independent DTC3-backed compressed PURE workload. Its
original KCORE/PURE/MRES, MM, VFS and user RUN/PIPE/WAIT source are
unchanged. The real installed KCORE reclaim JRST identifies the PURE
MRES base for the first FULL loader and service return breakpoints.
On its actual FULL loader entry (octal 036153), slot3 owns distinct
pinned PACKED RAW suffix 111200..120000 and complete temporary
PROCESS extent 120000..130000. DTC3 is detached; the loader returns
-4 at original service return 033067. Source and target cleanup leave
one unpinned original PACKED owner and no incomplete PROCESS target.

Before DTC3 is reattached, an independent physical breakpoint at the
permanent KCORE `proc_sched_pi_tick` entry 017205 executes. Its
first actual instruction is octal 350040024510, `AOS 1,proc_timer_clock`.
This is derived from original `proc_pdp6.s` and the source-linked
KCORE map (`proc_timer_clock=024510`), and independently verified
against the physically examined instruction. The physical clock
word is 021431 octal, cooldown 0100 and scheduler cursor 000003.
`proc_current_slot=0`, the original selected TSFS ROOT and every
original MM active extent, type, owner, size and pin exactly match
the snapshot after failed FULL rollback. No stranded RAW suffix is
unlocked as a runnable PROCESS APR.

The exact same private DTC3 reel is then reattached, all five child
mutable-data/PIPE `w` witnesses complete, and native EXIT/WAIT/HALT
finishes normally. The entire previous v1845 source-linked physical
audit is a prerequisite to the v1847 acceptance. Two independent
negative mutations of the real PI6 opcode and PACKED source pin are
rejected by the matching v1847 physical verifier.

Original source-matched image SHA256 values, unchanged from v1845:

* KCORE: `33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`
* PURE MRES: `ad246ce14079a821cd69f4c4f7b7661f5d1a0f58c118bdfa5cfa7adf2b2bfe3f`
* Accepted original physical trace: `67e462afa59f3e760440b01d6e9b2057a0a4e89147fb59918c73d740b62843b1`

## Rejected experiment and corrected test oracle

An initial testkit-only attempt awaited a second FULL callback while
the same DTC3 reel remained detached. Its later apparent MRES PC was
not a valid FULL invocation: captured AC2 (physical destination) and
AC4 (trusted extent length) were both zero, with no pinned complete
PROCESS target. A host timeout signal also terminated the experiment
before its eventual WAIT/HALT. The diagnostic was rejected and retained
outside the repository in a versioned $TMPDIR evidence directory.
One cannot assume an optional MRES's physical location is permanent
across later scheduler activity or that retry necessarily uses FULL
rather than PREFIX. A real second restore was not proven.

The replacement v1847 experiment observes the permanent KCORE PI6
handler instead. Its first test-only opcode oracle incorrectly read
KCORE DXR IMAGE offsets as permanent installed physical locations,
which gave a word different from the real AOS at physical 017205.
The verified source-and-map encoding corrected this test bug, and the
original unchanged simulator trace plus both source-linked mutants
were then reaudited to PASS. No production kernel code or RAM grew.

## Acceptance boundary

The actual interrupt entry is AFTER failed FULL source restoration
has completed its rollback, NOT an interrupt inside a pinned source
or target MM transaction. The run does not prove arbitrary PI6
interleavings, permanent unknown MM pins/free errors, transient peak
RAM, K06-K09 or production PURE enablement. K05 is still OPEN.
