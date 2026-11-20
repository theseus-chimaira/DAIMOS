# K02 permanent D6FS refusal and independent PIPE/HUP progress v1727

Date: 2026-11-20. Status: 256K PDP-6 physical guest and corrected
strict verifier replay PASS. K02 and K04 remain OPEN.

## Lifetime contract

The prior v1726 guest proved that two live RUN children can recover
independently from temporary D6FS and PIPE teardown refusals.
The new v1727 test asks what happens if a provider never releases
one child's FD, while another child must finish fatal HUP/EXIT.

Production proc_fatal_close_current_v879 keeps the original
process-private executive continuation and live FILE entry across
failed provider calls. It sleeps and doubles the retry interval from
one tick to a ceiling of 01000 octal (512 decimal). Existing
PROC_FATAL_CLOSE_BIT guards that sleeping owner against foreign
HUP/TERM attempting to release its private stack. Permanent refusal
means the original FD is still owned, not forcibly reclaimed.
Bounded CPU retry frequency and bounded resource reclamation are
separate properties. This test proves the former, not the latter.

## Reproducible full-system experiment

The new versioned daimos-testkit entry points are:

- tests/validation/daimos-k02-permanent-fatal-v1727.py
- tests/validation/daimos-k02-permanent-fatal-v1727.sh

The generator uses the accepted v1726 RUN parent/two-child test. It
creates C89 guest source and applies test-only anchored changes to
isolated copies of ipc/pipe.s, fs/d6fs_runtime.s, fs/fs_mres.s and
proc/proc_pdp6.s. Production KCORE, D6FS, FILE, MM, VFS, scheduler,
syscalls and device drivers are never modified.

Child B (slot2) inherits a regular D6FS file in FD6. The copied D6FS
provider hook recognizes precisely B's FD6 and returns an error for
every registered SYNC call, after normal MRES lease acquisition.
B's first ordinary EXIT fails and retains FD6. B then triggers an
actual APR protection fault by accessing an invalid user address.
The production fatal FILE owner runs indefinitely on its private
executive stack, sleeping between refused provider calls.

Only the disposable proc_pdp6.s has a probe at the existing fatal
FILE wait label. It checks the actual owner-stack retry interval
against 1,2,4,... up to 01000 octal and records sleep count, last
delay and any mismatches in three test-only resident KCORE words.
The production proc_sleep_ticks routine is used unchanged.

Child A (slot3) owns the last anonymous data PIPE reader and writer.
The test kernel physically pins the PIPE MM extent at first final
close. Real mm_free returns MM_ERR_BUSY. A then self-delivers HUP;
that fatal-close path unpins and frees the same PIPE exactly once.
At both A final-release attempts the PIPE hook reads the other
process's *actual* stable u-area: B's FD6 must be nonzero and
PROC_FATAL_CLOSE_BIT must remain set. It records two successful
overlap observations rather than relying on aggregate counters.

INIT waits for A's SYS_WAIT_EVENT_FLAG | SYS_EVENT_HUP status. It
checks that B is not waitable, attempts remote HUP against B and
requires refusal, sleeps another 0200 octal ticks, and verifies B
remains non-waitable. SYS_MEMINFO reports exactly one additional
occupied system-wide process slot for B. Its file_slots_used field
counts only *the calling process's* FILE table; INIT therefore
correctly has its original FILE count even while B retains FD6.
This is explicitly confirmed by syscall_info.s and checked
independently of the physical FD6 u-area observations.

## Physical 256K PDP-6 acceptance

| Observation | Result |
| --- | ---: |
| Permanently refused D6FS child | slot 2 |
| Independent PIPE/HUP child | slot 3 |
| D6FS SYNC calls / rejected calls | 9 / 9 |
| Correct serialized MRES lease observations | 9 |
| Wrong lease observations | 0 |
| Fatal-owner private-stack sleep calls | 8 |
| Exponential intervals observed (decimal ticks) | 1,2,4,8,16,32,64,128 |
| Fatal-owner delay mismatches | 0 |
| Last retry interval | 0200 octal / 128 decimal |
| PIPE final-release attempts | 2 |
| Real MM pin / unpin | 1 / 1 |
| Real MM_ERR_BUSY / accepted MM free | 1 / 1 |
| B FD6 + FATAL_CLOSE observations during A cleanup | 2 |
| Other ownership errors | 0 |
| Independent HUP completion and parent WAIT | PASS |
| Foreign HUP rejection and B WAIT_NOHANG | PASS |
| MEMINFO scoped counts and expected HALT | PASS |

Full test-only instrumented KCORE DXR SHA-256:
2d5dba3e34d1feb5ee3a781d1961e234f90c4b04f3609fdc54334141285e39a3

Full physical SIMH transcript SHA-256:
c073630689c79899d0f5f4b41fdbd08be973cf0a6040fe3ef64adacd808f6f14

The guest reaches its user-requested HALT and emits a combined BP
output witness. B writes one character before its first ordinary
EXIT closes stdout; INIT emits P later, and neither call inserts a
newline. The strict verifier was corrected to accept one exact BP
line, then replayed successfully against this preserved full guest
transcript. The original standalone runner had completed the
physical run but returned failure solely on that output-format
assertion; do not misreport it as an end-to-end positive script exit.

The verifier rejected four unsafe transcript mutations: deleting B,
setting a fatal-delay mismatch, changing a provider refusal counter
and corrupting the last retry interval. These are actual negative
tests of the acceptance assertions, not simulated production failures.

## Diagnostic fixes and independent native checks

Three validation-harness problems were found and corrected:

1. The early v1727 hook rejected references to its own new symbols.
   Checking existing definitions rather than all references fixed it.
2. A full physical run produced user !C because its test treated
   SYS_MEMINFO.file_slots_used as global. The resident syscall source
   proves the field is caller-local; the test now checks the correct
   scoped value and separately inspects B's physical FD6 owner.
3. The strict transcript verifier originally required separate B
   and P lines. The real console writes individual characters without
   newline; its actual combined BP line is now checked exactly.

These fixes did not alter or bypass the real fault mechanisms. In the
successful physical run the full owner/refcount/backoff witnesses
matched the expected values.

Related native tests on current production source:

- daimos-reaper-backoff-v1102.sh: retry backoff and unsafe mutants PASS.
- daimos-fatal-owner-hup-v970.sh: private-stack fatal owner / HUP PASS
  after repairing stale fixture symbols (testkit commit 2f0e557).
- daimos-pipe-typed-close-v1099.sh: typed MM refusal and four unsafe
  mutants PASS.

## Footprint and remaining open gates

Production KCORE instructions added: 0 words. Permanent resident
data added: 0 words. Per-process RAM added: 0. No new API, owner
registry, allocation, workaround or source change.

This verifies bounded retry CPU work and continued scheduler progress
while a real provider entry refuses forever. It does not prove
reclamation of that still-owned resource; K04 must define that policy.
Also unaccepted are genuine physical media faults and all remaining
concurrent session HUP/fatal EXEC/VM/PI interrupt permutations.
K02 is still OPEN, K03 OOM victim termination stays disabled, and
pdp10-doc is intentionally unchanged while K02 work is incomplete.
