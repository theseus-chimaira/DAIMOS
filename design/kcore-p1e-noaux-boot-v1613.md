# PDP-6 P1-E v1613: real 64K TSFS boot without AUXST1

Date: 2026-11-16
Base: accepted v1612 optional PURE MRES (GCC 1758, KCC 2196
installed PDP-10 words), default OFF.
Verdict: **physical 64K no-AUXSTORE boot/INIT accepted**;
real RAW pressure, restoration, immutable-source lifetime,
correct complete physical RAM accounting and net gain OPEN.

## Production boot-selection defect found and corrected

The original dedicated TSFS SWAP MINIT did:

    if (auxstore_boot_discover() != 0)
        return;
    if (auxstore_backstore_blocks == 0)
        install_optional_pure();

But AUXSTORE discovery returns -1 when *no valid descriptor*
exists. Consequently, a genuinely diskless TSFS root could
never install PURE: the first return preceded the optional
MRES selection. A formatted descriptor advertising exactly
zero BACKSTORE blocks had been the only possible install case.

Now the actual KINIT source treats either absent/unvalidated
AUXST1 (`discover != 0`) or validated zero capacity as a
no-BACKSTORE configuration. With the existing explicit
`DAIMOS_PURE_MRES_ENABLED=1`, that path installs the
source-selected PURE MRES and patches the two established
resident scheduler trampolines. KINIT prints `PURE LOADED`
only *after* successful module installation and export
validation. The diagnostic uses existing transient MINIT
output code, not permanent KCORE or MRES storage.

A positively validated nonzero BACKSTORE continues to
`auxstore_post_minits` and installs exactly the historical
DISK SWAP MRES and backstore service exports. A descriptor
read failure is never treated as permission to use disk
storage: no validated BACKSTORE owner exists. The original
default-OFF build continues to skip SWAP on a TSFS root
without validated backing.

No new physical driver probe, storage mode, runtime
dispatch, process descriptor, heap object or RAM registry
was introduced.

## Source-linked KINIT selection acceptance

`daimos-testkit/tests/validation/pure-boot-select-v1613/`
compiles the **actual** `proc_swap_minit_v481` function
extracted from `module_minit.c` as C89 with controlled
discovery and installer calls. PASS:

* TSFS/PURE enabled, discovery -1, zero capacity:
  install PURE and patch both resident scheduler entries;
* TSFS/PURE enabled, valid zero BACKSTORE:
  same PURE selection;
* TSFS/PURE enabled, positive validated BACKSTORE:
  install DISK and publish seven original service exports;
* TSFS/PURE disabled, absent or zero BACKSTORE:
  no SWAP/PURE MRES installation;
* D6FS root: existing DISK selection unchanged;
* a deliberately unsafe source mutation ignoring
  discovery -1 is rejected by the executed C89 tests.

The historical v490 compiled GCC SWAP MINIT regression
still passes.

## Complete physical boot evidence

The v1613 test creates a real three-member TSFS root and
boots the current GCC low-memory profile on a 64K SIMH
PDP-6. DTC0-2 contain the actual TSFS media. DSK0-3
and DR0-3 attachments are **removed from the active
simulator configuration**; a default generated auxiliary
image is not evidence of backing presence in this guest.

The real physical guest reports:

    DAIMON                              V0.5
    MACH                                PDP6
    MEM                                  64 K
    DTC                                   OK
    PURE                              LOADED
    DTFS                              LOADED
    INIT V1

The simulator remains running until the test's controlled
timeout; it does not halt after those boot markers. No
root-selection failure or INIT respawn error was observed.
The source-linked physical test is
`tests/validation/pure-boot-v1613/run-v1613.sh`.

Full 64K image KINIT `__kinit_image_end=0113622` octal;
fixed GCC `__kcore_low_end=024254` octal. Fixed KCORE DXR
is byte-identical to v1612 (SHA-256
`c6ff2b27165917059920f30570835bd1eddd008590fc4859b7d69600711e164a`).
The installed PURE module still costs **1758 GCC words**,
no BSS or per-process RAM. Physical boot reachability
is now verified, not merely a link or unit-test result.

## Pressure diagnosis is NOT accepted

The historical DISK swap test creates large process images
and expects a whole image to disappear. That is not the
correct test of smaller RAW PURE-prefix reclamation:

* A physical 64K test using 8193-word BSS per process
  reached `PURE LOADED` and `rapbp`, but the third
  pressure child RUN returned failure (`!e`).
* Repeating with 7169-word BSS yielded
  `ra[a045020/042522]pb[b067430/042522]`
  `[c112040/042522]p!e`. The third large child RUN
  again failed before the reclaim/wakeup assertion.

The snapshots are the *guest* octal
`SYS_MEMINFO.process_words` and
`SYS_MEMINFO.resident_words`. The first metric counts
only mapped user VM bases. A PACKED RAW process retains
a physical suffix MM extent even though its APR base is
zero, so the current metric does not include that
suffix. It is therefore not an exact physical RAM
measurement and cannot prove net savings.

These two attempts do not prove a kernel failure or a
working RAW scheduler transaction. The exploratory
script and reproducible failure evidence are kept under
`daimos-testkit/tests/diagnostics/pure-pressure-v1613/`,
not as a required passing validation.

The next pressure acceptance must establish a set of
smaller sleeping owners first, then request a controlled
incremental BRK/MM allocation to force RAW prefix DROP.
It must positively observe the exact retained physical
suffix, real slot-0 RESTORE and PIPE/WAIT completion.
Only actual MM extent accounting minus the installed
1758-word PURE MRES and transient high-water should be
used to claim a positive net RAM benefit.

## Remaining production release gates

* An executable source hold across the entire inactive
  interval, including unmount/source mutation failures;
* a true 64K pressure/PI6 DROP, RUN restoration,
  actual executor EXEC/EXIT/WAIT/BRK and fault matrix;
* accurate physical MM accounting of PACKED suffix
  (not just mapped-process `SYS_MEMINFO`);
* measured peak restoration memory and net RAM benefit;
* lower-memory 48K/32K boot and runtime gates if claimed.

Keep `DAIMOS_PURE_MRES_ENABLED=0` by default.
Compression, KA10 and KI10 remain deferred.
