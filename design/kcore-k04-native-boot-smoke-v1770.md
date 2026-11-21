# K04 v1770: native KCC staging and real PDP-6 boot smoke

Date: 2026-11-21. Status: full 256K production boot and clean reboot
smoke PASS. K04 fault-injection and recovery acceptance remain OPEN.

## Build blocker and source correction

The v1769 full `system` build passed, but the first fresh production
`system/boot/pdp6 image` failed while staging native KCC sources.
`scripts/stage-kcc-sources-v1.sh` requested the unversioned
`runtime/{crt0,daimos-bootstrap,syscall-helpers}.dobj`. The
authoritative KCC `install-native` recipe already installs these
objects as `*-v1.dobj`. DAIMOS commit `c1155cd6` makes staging use
the installed versioned names instead of introducing aliases.

The same staging contract also requires the native compiler's
`runtime/fp36-conversions-v1750.a` archive, built and linked by KCC
but omitted from its installation recipe. KCC commit `fe6dff8`
adds that archive to the native target and installs it alongside
the other bootstrap runtime objects. `make install-native` completed
and installed all required artifacts into `$PDP10_PREFIX`.

With these corrected producer/consumer dependencies, the fresh
production DSK image completed under
`$TMPDIR/daimos-k04-v1770-boot`. KCORE assembly and linking,
native userland, KCC source staging, disk/auxiliary media generation
and SIMH configuration all completed without changing the external
testkits. The image used a 256K PDP-6, disk ROOTSET=1, headless
SIMH graphics, real-time PCLK and distinct DCS/GE host ports.

## Actual emulator evidence

The unmodified SIMH PDP-6 booted the fresh image: `DAIMON V0.5`,
`MACH PDP6`, `MEM 256 K`, `HZ 60 LINE`, `D6FS LOADED`, and `INIT V1`
appeared on CTY, followed by `LOGIN:`. Logging in as `ROOT` reached
`DSH V1`. The `PS` command displayed columns `TD RETRY PVM DETAIL
ERR`, all zero for the normal live processes, and PID 5 for the
running PS command. `TRUE` completed and returned to the shell;
a further PS also completed normally, with no retained zombie.
Guest `HALT` reached octal PC `022464` and terminated the emulator.

Read-only `d6fsck -n 4 -d <diskdir>` then reported superblock 742,
sequence 3, CLEAN; 179196 blocks and 360 FCBs. An independent
clean reboot reached INIT, ROOT/DSH, PS and the same guest HALT PC.
After that boot the same read-only check reported sequence 5,
CLEAN, with the same block and FCB counts.

An earlier diagnostic run was forcibly stopped by host `timeout`
at the guest login prompt. Its disk superblock became DIRTY (743,
sequence 2), and the next boot correctly failed with early `?RT`
(root validation/mount failure). After regenerating only the
disposable test media, `d6fsck` reported CLEAN sequence 1 and all
subsequent guest-controlled boots and halts succeeded. Do not
mistake this dirty-media effect for a K04 teardown regression or
claim that externally terminated SIMH preserves a clean filesystem.

## Remaining K04 acceptance

This is real PDP-6 boot, normal process execution and diagnostic
smoke evidence, not physical fault injection of FILE/MM or a proof
of deferred finalizer correctness. K04 stays OPEN for temporary and
permanent refused owners, concurrent PREVM/post-VM owners, backoff
clock wrap, deferred session HUP, orphan WAIT/PID reuse, and an
owner-preserving operator recovery protocol. Both external testkit
suites remain untouched. No resident kernel state or code was added
by the v1770 boot-compatibility correction.
