# K05 v1890: original full-system PDP-6 physical FULL overpin refusal

Date: 2026-11-23. Status: **scoped source-built 48K physical PASS**.
K05-K09 remain OPEN. K01-K04 CLOSED. RAW PURE/mode3 stay OFF by default.

## Objective and original machine

v1855 fixed an actual FULL transaction bug where a second unexpected
MM pin on the temporary PROCESS APR survived one successful unpin and
could permit deletion of the authoritative PACKED suffix before
RESIDENT publication. Native KCC PDP-6 tests and a healthy five-child
whole-machine 48K boot passed, but the overpin fault had not been
injected directly into the original full-system PDP-6 MM table.

v1890 reuses the **unchanged v1855 48K** built original five-child
RUN/PIPE/TSFS-root image (one root plain, four compressed PURE backed
by independently attached DTC3) and **unchanged production source**.
The fixed KCORE SHA256 remains
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`;
the optional v1855 PURE MRES SHA256 is
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
The SIMH controller copies DTC0-DTC3 into private test media; no
original drive is altered. The immutable secondary DTC3 source is
**never detached** in this experiment.

The actual current machine's installed PURE base `032417` octal was
derived from a new physical KCORE reclaim JRST probe at `024143`
(jump `032627`), not guessed from a previous MRES load. The
source-linked bounded FULL loader is at `036202` octal and original
service return at `033067`. The original service reaches FULL with
the same two once-pinned slot-3 owners as the previous v1848 physical
reference:

* PACKED RAW suffix: `111200..120000`, type 4, pin **1**.
* Separate complete PROCESS destination: `120000..130000`, type 1,
  pin **1**.

The test controller verifies the actual pre-injection physical word
at KCORE MM table address `024705`: `000011000003` (target pin 1).
It deposits exactly `000021000003` (target pin 2), re-examines the
actual hardware word, then resumes the **unchanged** FULL loader.
This is external, test-only physical fault injection into the
simulated PDP-6; it is not evidence of a spontaneous MM pin leak.

## Original physical response and conserved owners

The actual FULL routine returns the 36-bit sign-extended
`777777777773` (= `MM_ERR_BUSY`, -5) at the real source-linked
service return PC `033067`. Full original MM table snapshots at
loader entry and at this return differ **only** by the injected
extra target pin. Both original reservations remain live: PACKED
pin 1, PROCESS pin 2. All 16 other MM entries, selected TSFS root,
current slot 0 and descriptor count (18 decimal) are unchanged.
There is no speculative unpin/free of the sole RAW suffix and no
successful RESIDENT publication return.

SIMH then executes a genuine later fixed-KCORE PCLK handler at
`proc_sched_pi_tick=017205`, with original clock/cooldown/cursor
octal `021676/0100/3`. A second full MM snapshot at that clock entry
matches the held one exactly, including both retained pin counts
and selected ROOT. The CPU has progressed past the failed FULL
routine without discarding the uncertain ownership state.

This purposely bounded physical run ends at a monitor clock
breakpoint, **not** the guest's final WAIT/Z/HALT. It proves a
physically retained owner pair after the actual failed FULL and
one later PCLK interrupt. It does **not** independently prove
user-level bystander WAIT/EXIT, recovery of a permanently corrupt
pin, correct physical terminal cleanup, all interrupt interleavings,
or K05 release acceptance.

The original normalized simulator transcript SHA256 is
`fa522cdbd0c1dfa8c429ba551d12db84d4671784928204f729c73eb8a0bb8b1e`.
Its original physical simulator exit and complete source-linked
verification **PASS**. The independent saved-trace auditor and
four separately executed negative controls PASS: forged pin1
pre-injection, missing physical pin2 after the deposit, absent source
pin on the failed return, and lost target pin at the later clock.
Every mutant is rejected by its intended original-MM witness check.

## Reproduction and boundaries

Testkit versioned harness:
`tests/validation/pure-k05-physical-overpin-v1890/`, including
`prepare-v1890.py`, `run-v1890.sh`, `verify-v1890.py`,
`negative-v1890.py`, `audit-v1890.sh`. Use
`make test-pure-k05-physical-overpin-v1890` to run the real
48K simulator and `make test-pure-k05-physical-overpin-audit-v1890`
for independent source/map/trace and mutation verification.
Run variables and exact original source/probe prerequisites are in
the testkit README. Preserved physical evidence is
`$TMPDIR/daimos-k05-physical-overpin-v1890-real/`.

**Follow-up v1891 scoped physical PASS:** a separate complete
source-linked run now reads and verifies the original slot3
TRANSITION bit, zero published VM base and unaffected nontransition
peer process descriptors after the held error and PCLK. See
`design/kcore-k05-process-quarantine-v1891.md` for that additional
independent physical oracle and its negative controls.

**Still OPEN:** unrelated child EXIT/WAIT while the owner remains
quarantined, other persistent MM allocator/free refusals,
physical owner uncertainty on terminal cleanup, and the complete
K05-K09 release matrix. No kernel source, KCORE/MRES image, static
data, process bit or production default changed in this experiment.
