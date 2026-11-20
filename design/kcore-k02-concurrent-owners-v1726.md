# K02 two-process overlapping FILE/PIPE teardown v1726

Date: 2026-11-20. Status: full-system 256K PDP-6 PASS; K02 OPEN.

## Goal

v1725 verified that one RUN child could retain an anonymous PIPE
extent and D6FS file across independent MM and provider refusals.
v1726 instead exercises two *simultaneously live* child process
owners, each with its own u-area, FILE table, VM, scheduler state,
refusal and ordinary EXIT. This also verifies that other processes
continue making progress while one child retains a refused owner.

## Reproducible physical guest

The new daimos-testkit files are:

- tests/validation/daimos-k02-concurrent-owners-v1726.c
- tests/validation/daimos-k02-concurrent-owners-v1726.py
- tests/validation/daimos-k02-concurrent-owners-v1726.sh

The user INIT program is compiled in C89. It creates a data PIPE,
one control PIPE for child B, one for child A and opens the real
regular D6FS file /SYSTEM/INIT. Two actual SYS_RUN calls produce:

- Child B, process slot2: control reader FD5, D6FS regular FD6.
- Child A, process slot3: data PIPE reader FD3, writer FD4 and
  control reader FD5.

INIT relinquishes its data-PIPE and D6FS file copies before starting
either child. It writes B's control byte first and sleeps 040 octal
ticks. B attempts ordinary EXIT: D6FS SYNC refuses, B retains FD6
and sleeps 0400 octal ticks on its own stable kernel/user state.
INIT then sends A's control byte. A attempts ordinary EXIT twice:
the first MM free is genuinely refused because the test kernel pins
its data-PIPE extent, and the second unpins and frees exactly once.
INIT WAITs for A's exit status 051, then closes its control writers
and WAITs for B's exit status 052 after B's second refused SYNC and
successful third EXIT. SYS_MEMINFO verifies no process or FILE leak.

The parent retains both control-PIPE writer references until A has
completed. This avoids selecting a control PIPE as the instrumented
first last-reference MM object; only the data PIPE is targeted.

## Exact physical overlap assertion

The test-only PIPE assembly is copied from the accepted v1725 guest
and extended only in its disposable source. At BOTH data-PIPE final
release attempts it requires:

1. Current process slot is exactly A=3.
2. The copied resident D6FS provider counters show exactly one
   targeted provider SYNC call and one rejection.
3. The actual proc_table slot2 descriptor still has a stable u-area
   physical base, and its FILE table FD6 (u-area offset 042 octal)
   is nonzero. B therefore still owns its original refused FILE.

Each successful overlap observation increments a separate KCORE
test-only word. Both attempts must satisfy the requirement; the test
rejects either a wrong owner or premature cleanup. The PIPE itself
still uses real mm_pin, mm_free and mm_unpin; no MM service is mocked.
The D6FS provider refusal occurs at its real MRES SYNC entry with
the ordinary fs_provider_ready lease held. These errors remain
deliberately injected inside the provider, not physical media errors.

The complete guest also checks both independent parent WAIT statuses
and pre/post process, MM and FILE occupancy, which rules out a test
that merely reaches HALT with retained zombies or leaked descriptors.

## Observed 256K PDP-6 acceptance

| Physical witness | Value |
| --- | ---: |
| D6FS FILE owner slot | 2 |
| PIPE MM owner slot | 3 |
| Physical data-PIPE base (decimal) | 21,611 |
| Data-PIPE final-release attempts | 2 |
| Real MM pin / unpin | 1 / 1 |
| Actual MM_ERR_BUSY refusals | 1 |
| Successful data-PIPE frees | 1 |
| Observations of B's live FD6 during A close | 2 |
| D6FS provider calls / injected refusals | 3 / 2 |
| Correct provider lease observations | 3 |
| Wrong provider lease observations | 0 |
| Other ownership/refcount violations | 0 |
| Child A/B exit status and both parent WAITs | PASS |
| Process/FILE/MEMINFO baseline | PASS |
| Exact parent P marker and expected HALT | PASS |

Test-instrumented KCORE DXR SHA-256:
87bb46f475fb11186e194c476a454b2512ba39d09f2a2f2ca4de46f5571014ba

Physical SIMH transcript SHA-256:
bbf57b3918373951e735e73d01bee3fcb2354abf688591872a7f791f9ad0182f

The final versioned physical guest completed with exit status zero.
The full simulator verifier checked all the exact counter identities,
two overlaps, both owner slots, zero errors and user-requested HALT.
Its unmodified production KCORE code is unchanged from v1725.

## Open limits and memory footprint

Production KCORE instructions added: 0 words. Production fixed data
added: 0 words. Per-process RAM added: 0. All diagnostic counters
and injected refusal code belong to the disposable test guest only.

This verifies overlapping live owners on a booted single-CPU system,
not literally simultaneous execution of two PDP-6 instructions.
It does not validate spontaneous media faults, permanent provider
refusal, HUP/fatal EXIT while a provider operation remains suspended,
or every interrupt and slot-zero finalizer interleaving. Those remain
open K02/K04 acceptance gates. K03 OOM killing stays disabled.
Documentation under pdp10-doc remains unchanged while K02 is open.
