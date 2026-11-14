# KCORE P1-C v1577: LOGSTORE torn-record refusal and cursor rollback

Date: 2026-11-14. **Accepted P1-C fault-isolation increment**;
the combined P1-C task remains OPEN. P0-A and operator-trust P0-B
remain CLOSED. VERSION 0.5, strict 32K/optimization HOLD.

## What is proved

The original LOGSTORE MRES implements append as a 128-word record
write followed, only on successful backing completion, by sequence
and ring-slot advancement and producer-event notification. The
record carries a final commit trailer, distinct from its magic and
sequence header. A failed write must leave the next sequence and
slot unchanged so that subsequent append can retry the same record.

Two separate PDP-6 test classes now verify that behavior:

1. Source-linked native `p10run` invokes the **actual production**
   `logstore_mres_dispatch/logstore_append/logstore_write` on a
   test-controlled 128-word backing medium. The first write copies
   the record, clears the commit trailer and reports -1. The native
   oracle verifies the physical slot and sequence, invalid trailer,
   original cursor, zero event/wakes, then performs an accepted retry
   of the exact same slot. It requires one sequence/slot advancement,
   exactly one wake and real MRES READ_BLOCK of the committed record.
   Four executed unsafe assembler mutants fail: ignore write status,
   skip sequence advancement, skip slot advancement, and skip wake.

2. Full physical KCC 128K PDP-6 kernel with Type-236 DRM236 backing,
   permanent MEMFS and LOGSTORE on the **same AUXSTORE drum**, booted
   twice with fresh CPU RAM on boot 2 and the original 8 MiB image.
   An isolated test-only LOGSTORE MRES changes only the first APPEND
   of one test-specific event code on each boot. It physically writes
   a real 128-word sector containing the right magic, sequence and
   payload but **zero commit trailer**, then reports a synthetic
   error to the untouched production append/rollback code. The guest
   checks negative APPEND, unchanged STATUS sequence/ring slot and
   the actual invalid physical sector through raw LOGCTL READ_BLOCK;
   normal retry physically re-writes the same slot with a valid
   trailer, and only then advances the cursor. In both boots, MEMFS
   persistent data/XIP remains correct and HALT succeeds. On boot 2,
   the guest first discovers and validates boot 1's committed LSREC1
   event and restores the MEMFS snapshot. Every backing descriptor
   remains byte-identical after both shutdowns.

Full physical guest verdict:

```
PASS v1577: failed physical uncommitted LOGSTORE record kept original cursor
PASS v1577: real same-slot retry, new boot old record, MEMFS snapshot restored
```

## Implementation footprint and limits

**Zero production code or RAM change**: the new event-specific
failure hook and two tiny counters exist only in a disposable copy
of the optional LOGSTORE MRES, under daimos-testkit. In particular,
KCORE's fixed low end remains `024364` octal. No alternate storage
layer, new MRES API, format change or HALT workaround was introduced.

The simulated completion error happens **after an actual successful
physical write of an intentionally invalid sector**. It establishes
failure-state handling for a physically torn/uncommitted record,
but it does NOT demonstrate a naturally failing Type-236 controller
or sub-sector failure by the hardware, and the immediately retried
record is valid before the next cold boot. Recovery of an abandoned
orphan-invalid record, simultaneous process swap-in/out during
MEMFS commit, and actual in-flight DEVICE/VOLUME/PI/BCACHE faults
remain unaccepted P1-C work. P1-D/E remain OPEN separately.

Testkit authoritative physical source and probes:
`tests/validation/daimos-p1c-logstore-torn-v1577.sh/.py/.md`,
`tests/validation/daimos-logstore-fault-rollback-v1577.sh/.s`.
