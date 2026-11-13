# KCORE v1530: independent physical test media and SIMH scripts

Date: 2026-11-13. KCORE feature validation infrastructure;
no change to production KCORE, DTFS/TSFS MRES, simulator
or on-media format. The existing v1529 physical partial-write
result is documented separately in
`design/kcore-dtfs-partial-write-physical-v1529.md`.

## Correctness bug in the physical test harness

The v1525-v1529 full PDP-6 guest runner accepted a reusable
full-KCC image but then wrote new root TSFS tapes into its
`build/media/dtc0.tap` through `dtc2.tap`, re-created
`build/media/dtc3.tap` and `dtc4.tap`, and rewrote
`build/boot.headless.ini` with per-run terminal ports.
Two independent guest runs using the same prebuilt image
could consequently overwrite each other's **actively
attached physical tapes**. The v1529 host media verifier
observed one such conflicting DTC3 seed. It was rejected
as an invalid test, not misdiagnosed as KCORE damage.

Simply allowing longer I/O timeouts or retrying the
same shared images would not correct this hazard.

## Testkit repair v1530

The existing `daimos-dtfs-tape-root-mount-v578.sh`
now creates uniquely named writable `dtc0-v1530.tap`
through `dtc3-v1530.tap` in its process-private
`$work/media`. If applicable, native DTC4 and the
replacement medium are there too. The disposable TSFS
root is generated directly into private tapes. For
variants not rewriting root, original tapes are
copied rather than attached writable to a common
prebuilt location. Native `mkdtfs` creates DTC3 only
in the private directory, with its genuine nonzero
72-bit volume ID.

The SIMH headless command file is generated as
`$work/boot-headless-v1530.ini`, never in the
shared KCC build. It retains the original boot
commands, but rewrites four exact DTC0-DTC3
attachment pathnames to the private tapes.
Physical DETACH/ATTACH operations, first-word
preimage checks and all 128-word postimage
comparisons consistently use those same paths.
The helper's CTY/DCS setup, guest executable,
provider I/O, PID1 and controller remain real.

This makes the prebuilt full KCC image immutable
from the viewpoint of every physical gate while
keeping original boot semantics. The fix adds
no mock hardware, extra controller event timing,
permanent memory, simulated fault, KCORE hook or
new filesystem mechanism.

## Complete regression

The following booted 256K PDP-6 guest gates
were rerun on the corrected fixture and PASSed
with their original strict assertions:

* v1529: real Type-551 DTC3 RBN0145 partial
  36-bit write, one new word and one old
  word verified after physical DETACH, sticky
  WRITE_UNCERTAIN and retained FILE ownership.
* v1528: actual DTC3 write failure, same-identity
  return still refuses FILE/FSINFO/UNMOUNT.
* v1527: DTC3 different stamped tape refused,
  DTC4 unaffected, original tape return verified,
  original FILE usable, both mounts retired.
* v1526: two independent processes, child cwd and
  FILE owners prevent premature mount retirement.

Native v1529 source-linked new-file 1/2-word
resize and real Type-551 RBN0145 write/read
also PASSed independently.

P0-B partial physical write fail-closed
containment is accepted from v1529.
**Explicit checked administrative repair or
discard of WRITE_UNCERTAIN mounts remains OPEN.**
P0-A and P1-C/D/E are unchanged.

Production size remains v1527 optional MRES
octal `003747` text, `000002` BSS; fixed KCORE
unchanged. VERSION 0.5, strict 32K HOLD,
no optimization.
