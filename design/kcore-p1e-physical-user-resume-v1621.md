# P1-E v1621: restored PDP-6 user code and pressure-fit negative control

Date: 2026-11-16
Base: pushed v1620 no-AUX 48K TSFS/PURE prefix commit, pin
release and scheduler READY_ONCE/current-slot acceptance.
Status: actual restored child user instruction sequence confirmed.
Complete four-child PIPE/WAIT/EXIT and positive physical net/peak
RAM savings remain OPEN. Production KCORE and MRES unchanged.

## Exact guest and original user-code execution

The v1621 replay reuses the *same* source-built opt-in 48K
PDP-6 guest, matching its TSFS root manifest entry for
/SYSTEM/INIT to the preserved user INIT DXR2 marked PURE.
The guest uses the v1615 typed-pressure variant of the
original testkit C child/pipe/BRK program. No native
guest instruction, removable module or kernel code changes.

The PDP-6 is stopped first at the original MM commit PC 034614.
Live write-watchpoints prove the in-place MM metadata states
PACKED pin1 -> PROCESS pin1 -> PROCESS pin0 at 024307.
The original PURE service writes READY_ONCE|slot2 (001002)
at PC 032477, and fixed KCORE selects current slot 2
at PC 016770. The child then actually resumes after the
last write-watchpoint is removed.

The physical console prints a standalone lower-case 'w'
**after** current slot 2 selection and **before** the
first of two bounded CPU continuation intervals expires.
The preserved booted C source has exactly one user-code
path printing 'w': the child returns from dsys_read_words,
checks both private writable-data sentinels and prints
'w', then requests exit with status 057 octal. Thus
restoration has led to actual successful user execution
and a completed pipe read with intact mutable contents.
No other packed child was runnable at the prior stop.

Following that marker, the CPU is observed in slot 0
servicing another DECtape search. It is NOT correct to
call this complete four-child WAIT or parent success.
The two later snapshots do not establish all child
exit statuses or successful parent BRK growth.

The v1621 testkit audit checks both the preceding
v1620 physical MM/scheduler write sequence and the
single user output marker against the actual saved
source, manifest and PURE executable. It rejects
test-only removal of the user marker or substitution
of an unrelated source. No new production MM or
process metadata and no KCORE/MRES size growth.

## Fitting BRK alone does not prove RAW reclaim

The historical source-built v1615 parent had staged
growth and requested a final 020000-octal BRK while
the measured largest aligned free run was 017000;
that target could not fit in the measured geometry.
The original parent's BRK failure must not be
confused with a failed *child* PURE restore.

An independent v1621 physical 48K TSFS/no-AUX
variant of the original v1614 four-child PIPE/WAIT
program instead requests BRK target 016000 octal.
It executes a real native BRK successfully:

| Measurement (octal) | Before | After |
|---|---:|---:|
| Parent BRK | 010000 | 016000 |
| Logical process words | 052450 | 060450 |
| Resident overhead words | 042502 | 042502 |

The logical process growth 006000 octal equals
the entire BRK increment. There is no net decline
in mapped process words to credit as RAW reclaim.
The original program therefore correctly rejects
the case with '!R'. Four 'w' child completion
markers arrive only after parent failure cleanup
releases its pipe. This is a **negative control**,
not a passing full PIPE/WAIT/EXIT workload and
not a physical net memory savings measurement.
The testkit checks the exact source variant,
physical guest readings, fail-closed 'R' assertion
and an unsafe modified-growth mutation.

A second rebuilt physical 48K/no-AUX variant
requested six children and the same fitting
016000 BRK. The console emitted five 'a'
child-ready markers and then !c *before*
the parent's BRK request. The original source
prints !c if RUN returns an invalid pid OR a
spawned child fails to reach PROC_SLEEP within
its bounded polling loop; the present output
does not distinguish these causes. It is
incorrect to label this a proven allocator
failure. A source-linked testkit negative
control confirms five accepted children,
sixth refusal and absence of any BRK attempt,
and rejects an altered refusal marker.

This six-child scenario is also not an
accepted end-to-end workload. A future
five-child experiment could establish a
working population before pressure, while
additional guest instrumentation would be
needed to classify the sixth-child refusal.
More packed owners are not evidence of
savings until actual physical MM descriptors
and memory cost are measured.

## Still-open acceptance

The installed GCC PURE MRES occupies 1758 decimal
words. Four simultaneous omitted 001000-octal-word
prefixes would represent 4 * 512 = 2048 decimal
gross saved words, a tentative 290-word advantage
**before transient memory or other costs**. After
one child is restored, three remaining omitted
prefixes total only 1536 decimal words, below the
removable module's installed footprint.

The next acceptance experiment must select a
truly pressure-inducing yet allocatable parent
BRK, verify physical MM owners at peak and after
every restore, observe all child PIPE/WAIT/EXIT
statuses and source lifetime, then measure exact
peak and net installed physical RAM (including
module, stacks and auxiliary dynamic allocations).
48K and 64K profiles need separate acceptance.

This v1621 work changes diagnostics and source-
controlled guest experiments in daimos-testkit
only. No production KCORE/PURE MRES/driver/MM/ABI
change; DAIMOS_PURE_MRES_ENABLED default OFF,
VERSION 0.5, strict 32K HOLD, no compression.
