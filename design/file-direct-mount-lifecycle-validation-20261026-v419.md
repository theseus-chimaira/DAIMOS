# Proposal B v419: process-lifecycle and RUN rollback acceptance

Status: both full PDP-6 simulator regressions PASS against committed v417.

## Test configuration

Used the existing daimos-testkit tests with `MAKEFLAGS=COMPILER=gcc`,
matching the supported strict DSH image-size configuration while
retaining the test's kernel boot profile and its production image limit.
Neither test fixture nor production kernel needed modification.

## Results

1. `test-daimos-process-lifecycle` — PASS: RUN, GETPID, WAIT/nohang,
   zombie reap, orphan adoption and explicit descriptor maps.
2. `test-daimos-run-late-rollback` — PASS: sixteen post-VM descriptor
   mapping failures under three live children, stable process
   accounting and successful recovery.

These simulator tests extend focused native FILE, directmount, PIPE
identity and VFS UMASK9 coverage beyond the v417 cutover. They do not
prove all failure-injection or filesystem durability cases.

## Size and status

No production change was needed; accepted full linked v417 GCC PDP-6
lowmem measurements remain KCORE=8999, KCORE+MRES=15547 and
PERMANENT_LAST=15594 words, a 110-word reduction over v388.
Physical per-process u-area size is unchanged. The strict 32K
acceptance task and unrelated JRST folding remain deferred.

Next acceptance focus: mount/unmount provider lifecycle and real
filesystem boot/read/write behavior under the six-bit descriptor
encoding; avoid re-running costly lifecycle tests without a relevant
change.
