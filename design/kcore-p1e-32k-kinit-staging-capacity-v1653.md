# P1-E v1653: original 32K KINIT transient capacity barrier

Date: 2026-11-17. **Source-matched original 32K
PURE ON and OFF KINIT build rejection verified PASS.**
Neither build reached a bootable image or PDP-6
simulator. No production kernel, MM, KINIT, MRES,
compressor, loader, ABI or stack-reserve changes.

## Genuine original matched configurations

The testkit built an independent minimal C89 PURE
user DXR2 with one-child RUN, private initialized
writable state, MEMINFO, BRK, EXIT-057, exact PID/
WAIT and normal EXIT. The user DXR2 compiled and
linked successfully. Both original GCC/lowmem
KINIT configurations used exactly the same DXR2,
32K PDP-6 target, ROOT=tape, BOOT=dtc,
TSFS_ROOT_MEMBERS=3, TSFS_INPLACE_BOOT=1,
PROC_BOOT_USERS=1, BOOT_CORE_WORDS=0100000 and
SIMH_CPU_KWORDS=32. Only the original
DAIMOS_PURE_MRES_ENABLED option differed:
0 for OFF and 1 for ON. Both runs reached the
original link and original image safety gate,
which refused to construct a boot medium.

The actual original KCORE link maps are byte-identical;
the original KCORE streams are also byte-identical.
Both have __kcore_low_end=024253 octal. The
PURE ON KINIT map includes the real removable
proc_pure_mres_package_v1612; PURE OFF does
not. Other original removable-package symbols
including swap, DTFS and MonitorFS are present.

The original physical 32K core limit is octal
0100000, decimal 32,768 words. The unchanged
KINIT bootstrap stack reserve is 0400 octal,
decimal 256, giving an original image end bound
of octal 077400 (decimal 32,512).

| Original build | KINIT end (octal) | Above physical core (decimal) | Above stack-safe bound (decimal) |
| --- | ---: | ---: | ---: |
| PURE OFF | 110052 | 4,138 | 4,394 |
| PURE ON | 113621 | 6,033 | 6,289 |

The unchanged ON/OFF transient KINIT increment is
exactly 03547 octal, 1,895 decimal words. This
is NOT a permanent KCORE increase: both KCORE
streams/maps match. The original OFF baseline
already exceeds even the total physical core
by 4,138 words; PURE is not the sole cause.
Both links exceed physical core itself, so a
reduced stack reserve could not make either
uncompressed image runnable.

The original mkimage safety check correctly
rejects both before media construction. There
is no legitimate 32K guest PASS from these
two runs. The original accepted 48K/64K
P1-E PACKED state tests are unaffected.

## Negative control and acceptance

The testkit compiled the original minimal C89
user executable, retained both original source-
matched KINIT/KCORE linker maps and genuine
mkimage error logs, and ran a separate capacity
oracle on the linked results. It verifies
byte-identical permanent KCORE, correct installed
PURE package selection, original 32K/stack
limits, both genuine error records and all
numeric bounds. Three independent forged
copies of the original evidence must fail:

* A fake in-range KINIT image end in the ON map.
* A fake success/improved link in the ON build log.
* A changed KCORE stream falsely claiming the
  ON/OFF cost affected permanent KCORE.

All three are rejected; no production source is
edited. Testkit
tests/diagnostics/pure-32k-process-v1653/,
branch test-kcore-p1e-32k-kinit-capacity-v1653-20261117,
commit 9b5a512.

Evidence: $TMPDIR/daimos-p1e-v1653-evidence-20261117.tar.gz.
SHA256:
75a59f34bfa2403e79378e257e4073010dcb13ef590e8a4ebc4f4d1a4967ebf1.
Contains both linked KCORE/KINIT map sets,
KCORE stream identity checks, original C89
user DXR, both build refusal logs, accepted
and rejected checks and versioned patch SHAR.

## Architectural implication and next gate

Current 32K PURE acceptance is BLOCKED before
RUN or MM testing by the transient KINIT image,
not by a detected allocator, assembler or
compiler defect. To admit this exact 32K feature
profile, a correct staged-package installation
or genuinely smaller transient KINIT lifetime
must reduce original simultaneous required
physical space by **at least 6,289 words**
relative to PURE ON, preserving the 256-word
stack reserve, complete required removable
modules and source-backed loader. Source
recompression alone does not prove that
runtime KINIT will fit after decompression.
No such redesign is implemented by v1653.

The original complete 32K machine boot, RAW
reclamation, transient FULL peak and KA10
backend remain OPEN. PURE stays OFF by
default and strict 32K whole-KCORE size
optimization remains on HOLD until feature
acceptance/freeze.
