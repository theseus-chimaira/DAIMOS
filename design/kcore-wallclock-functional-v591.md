# KCORE wall-clock functional integration audit v591

Date: 2026-10-28. Status: implementation pending; no runtime change.

## Existing code and compatibility

- `system/kernel/drivers/pclk.s` directly issues DATAI/CONI/DATAI to
  device 0730 and returns a packed TIME36. The routine currently assumes
  PCLK is present and retries across minute changes.
- UUO 077 extension 023 jumps directly to `pclk_time36` in
  `system/kernel/proc/syscall_dispatch_pdp6.s`.
- The D6FS and MEMFS timestamp paths also call `pclk_time36` directly.
  Consequently a GETTIME-only fallback would leave filesystem mutations
  dependent on absent hardware.
- `clk_pi_service` in `drivers/clk_io.s` distinguishes a qualified APR
  line-clock flag from a software reschedule request, and increments
  `clk_tick_count` once per real tick. The CLK package is installed as an
  always-resident MRES by `clk_minit` only when APR clock probing succeeds.
- `userland/exec/commands.c:cmd_date` currently accepts display only.
- `pdp10-doc/KERNEL/CLOCK.MD` describes an older PCLK-required contract;
  the accepted functional change in `TODO.MD` supersedes that limitation.

## Required implementation order

1. Establish native PDP-6 PCLK-presence detection semantics from hardware
   and both simulators. Do not issue an unguarded device operation during
   boot discovery. Explicitly test PCLK disabled and enabled.
2. Introduce one authoritative wall-clock TIME36 entry used by GETTIME,
   D6FS and MEMFS. Do not duplicate independent calendar states.
3. Store the fallback calendar as one atomic 36-bit TIME36 word plus a
   bounded subsecond accumulator. A qualified APR clock tick advances
   exactly once; software PI kicks must not advance civil time. Handle
   50/60 Hz via the actual configured hardware rate, not a fixed divisor.
4. SETTIME requires root credentials, validates every TIME36 field and
   supported year range, and atomically replaces the calendar and tick
   fraction. Failed validation leaves old time intact. Do not adjust
   scheduler `clk_tick_count` or sleep deadlines.
5. Preserve the PCLK fast path on PCLK-equipped installations. Define and
   test what SETTIME means when PCLK is present (either supported device
   setting or explicit rejection); do not report success for a write that
   the physical clock ignores.
6. Extend DATE with strict UTC parsing and leap-year/day validation while
   preserving its no-argument display behavior. Keep parsing in userspace.
7. Native daimos-testkit: PCLK-present/absent, 50/60 Hz, month/year/leap
   rollover, invalid date, non-root denial, concurrent GETTIME/SETTIME,
   timestamp propagation to D6FS/MEMFS, and scheduler tick independence.

## Acceptance policy

The UTC wall-clock feature stays open until all callers use the safe
interface and native coverage passes. No size saving, new resident object,
or supported PCLK-absent mode is claimed by this source audit.

## 2026-10-28 v592 incremental PCLK reader guard

SIMH's PDP-6 `null_dev` returns zero for DATAI/CONI of an unassigned
device. The reader now recognizes a zero initial DATAI and returns the
documented unknown TIME36 value instead of decoding it as a date. This
is an interim safety guard only: it does not prove physical-hardware
presence detection and does not provide a software clock or SETTIME.
The lowmem `permanent-size` profile links successfully; the full/auto
profile still fails its known permanent address limit. Native enabled/
disabled device execution must be tested before claiming acceptance.

## 2026-10-28 v593 native proof

The versioned `daimos-testkit` fixture
`tests/validation/daimos-pclk-presence-v593.sh` builds one PDP-6 system
image with a dedicated INIT program and boots that image with PCLK
enabled and disabled. The enabled case returns a valid TIME36 calendar
via GETTIME; the disabled case returns TIME36 zero. Both native runs
PASS. This proves the guarded SIMH device behavior, not the absence
probe's safety on unspecified physical PDP-6 hardware. The software
calendar accumulator, SETTIME and DATE SET remain unfinished.

## 2026-10-28 v604 tested UTC clock and SETTIME acceptance

The shared TIME36 entry now returns PCLK when available and the software
APR-tick accumulator otherwise. Privileged SETTIME uses UUO-077 extension
037, avoiding PROCCTL SETGID's reserved 025 extension. DATE can prompt for
UTC, and privileged LOGIN invokes DATE only when the clock is unset.

Native PDP-6 tests: v596 (PCLK present/absent SETTIME), v597 (ROOT LOGIN
invokes DATE, then DSH), and v604 (leap-day rollover, invalid calendars,
non-root SETTIME denial and PCLK-present refusal) PASS. v604 snapshots and
restores the pristine boot media before each simulated PCLK mode; earlier
repeated boots against mutated media hit `?RT` before INIT. Separate boot
forensics remain open rather than treating a clean rebuild as a repair.

Targeted v602 SIZEOPT.MD-style resident syscall change: the existing
`file_check_root` preserves AC2, so SETTIME no longer spills the TIME36
argument to the stack. Same lowmem permanent-size profile before/after:
packaged KCORE+MRES 15983 -> 15980 words; PERMANENT_LAST 16030 -> 16027
decimal (octal 037236 -> 037233), **three linked words saved**.
No claimed general KCORE optimization and no independent RAM increase.

Open gates: actual 50/60-Hz configuration, physical PDP-6 PCLK discovery,
additional concurrent time-setting/stamp stress and boot reuse forensics.

## 2026-10-28 v606 line-frequency discovery audit

Native PDP-6 APR `CONI` reports the clock flag and enable state, not the
frequency of the physical AC line. `clk_minit` probes only those states;
`minit_diag_hz` prints a hardcoded `HZ 60 LINE` label. The PDP-6 SIMH
`rtc_srv` delivers the same APR tick regardless of the internal `rtc_tps`
setting; its default PDP-6 build rate is 60 ticks/second. There is no
hardware-visible 50/60-Hz selector in the inspected device ABI.

**Conclusion:** Automatic 50/60-Hz determination is impossible from the
APR tick stream alone without an independent calibrated time reference.
No speculative tick-based auto-detection is permitted. The current
software wall clock remains explicitly 60 Hz; a 50-Hz implementation
must obtain its rate from verified platform/boot configuration or a
separate trustworthy reference and cover both cases in native tests.

## 2026-10-28 v607 installation-time frequency setting

The operator specifies the known physical APR line frequency at build time:

    make -C system/boot/pdp6 image WALL_CLOCK_HZ=50
    make -C system/boot/pdp6 image WALL_CLOCK_HZ=60

The default is 60. Values other than 50 or 60 fail at make time and in
the C preprocessor. The selected constant is propagated to the KCORE
wall-clock compilation, and changing it invalidates the generated
`wall_clock.s` even when reusing the same build directory. No mutable
frequency word remains in permanent kernel RAM. Cross-compilation was
verified for both rates: generated PDP-6 DIVI/MULI divisors are octal
062 for 50 and 074 for 60; invalid 49 fails. Both lowmem links pass.

The setting changes software UTC accumulation only; other historical
60-Hz scheduler, sleep and device-timeout policies are not silently
retimed. Their 50-Hz audit remains a distinct acceptance requirement.

## 2026-10-28 v608 native rate regression

`daimos-testkit/tests/validation/daimos-date-hz-v608.sh` builds separate
PDP-6 images for `WALL_CLOCK_HZ=50` and `60`. Each boots with PCLK off
and on using pristine restored media. Native acceptance PASS for both:
APR tick-to-UTC rollover, invalid calendar rejection, non-root SETTIME
denial and hardware PCLK non-writability. The test uses the selected
frequency as its elapsed tick count. SIMH's physical interrupt stream
still operates at its built-in 60 Hz, so this verifies configured tick
conversion, not an externally timed physical 50-Hz clock.

SIMH's optional DPY SDL window crashed before INIT during the first
attempt. The fixture disables the unrelated DPY device rather than
altering clock or display production code; the retained failing image
was used to confirm boot with DPY disabled. Both final runs PASS.

## 2026-10-28 v609 D6FS UTC modification timestamps

Testkit `daimos-date-mtime-v609` PASS at 60 Hz with PCLK present and
absent. The PCLK-absent fixture initializes UTC using SETTIME, creates
and writes a D6FS file, verifies its nonzero UTC mtime, advances across
leap-day midnight, writes again, and checks the updated mtime. PCLK
presence continues to give nonzero filesystem timestamps. No production
code change was necessary.

The same native fixture was additionally run with `WALL_CLOCK_HZ=50`:
PASS in both PCLK configurations with the same D6FS mtime and midnight
rollover checks. This validates the 50-tick configured conversion, not
SIMH physical 50-Hz interrupt generation.

Source concurrency review: `wall_clock_now` reads TIME36, the tick base,
and counter under PI masking and checks the base/value again before
publishing an advanced calendar. `wall_clock_set` changes TIME36 and its
base together under PI masking. This protects against torn state and
stale publication, but a multi-process native GETTIME/SETTIME stress
test remains open and is not claimed as passed.

## 2026-10-28 v610 MEMFS UTC modification timestamps

Testkit `daimos-memfs-mtime-v610` PASS at 60 Hz with PCLK enabled and
disabled. It verifies MEMFS file mtime creation and mutation after
SETTIME and across leap-day midnight. Source review found that
`memfs_touch_slot` carried its destination pointer in caller-scratch
AC5 across `wall_time36`. Although the current GCC assembly incidentally
restores AC5, the production caller now saves and restores the pointer
explicitly. This is an ABI correctness fix, not a claimed size saving.

## 2026-10-28 v612 multiprocess GETTIME/SETTIME acceptance

Native testkit `daimos-date-race-v612` PASS for independently built
`WALL_CLOCK_HZ=50` and `60` systems with PCLK absent. A privileged
parent repeatedly alternates valid leap-day-boundary TIME36 settings
while three separate child processes read GETTIME under normal scheduler
preemption, validating decoded fields and child exit status. This tests
the PI-masked snapshot and guarded publication paths in a multiprocess
setting. No production clock change or additional RAM was needed.

This bounded native race closes the targeted concurrency acceptance gate;
it does not constitute exhaustive indefinite stress testing. Physical
PCLK presence semantics and device/scheduler timing on actual 50-Hz
hardware remain independently open.

Additional native v610 rerun with `WALL_CLOCK_HZ=50` PASS in both PCLK
modes, including MEMFS mtime across leap-day midnight. This closes the
50/60-Hz MEMFS timestamp regression, not multiprocess clock stress.

## 2026-10-28 v594 unified TIME36 gateway

Introduced `wall_time36` as the single entry for GETTIME and D6FS/MEMFS
timestamps, including the provider C path. The original `pclk_time36`
continues to read physical PCLK and the gateway currently tail-jumps to it.
This preserves timestamp semantics while allowing a future fallback
calendar in one place. No software fallback or SETTIME is enabled yet.

## 2026-10-28 v616 KINIT provider selection (first stage)

KINIT `clk_minit` performs a single PCLK probe using the coherent hardware
reader and checks the full supported TIME36 calendar (including leap days).
It patches the one-word KCORE `wall_time36_provider_jump` to the hardware
reader only when its UTC value is valid; the default jump leads to the
APR-backed software calendar. Thus GETTIME and filesystem timestamp reads
never probe unavailable PCLK repeatedly. Native testkit
`daimos-date-hz-v608.sh` PASS at 60 Hz in PCLK-on and PCLK-off modes.

**Stage boundary:** this selects the provider at boot but does NOT yet
place each provider in a separate MRES or omit the unselected code from
permanent RAM. Both providers still reside in linked KCORE. Source removal
and mutually exclusive MRES packing require their own linked-size gate.
This stage preserves existing software SETTIME behavior; further edits
must not remove its privilege validation. The APR MRES scheduler remains
shared in all cases.

## 2026-10-28 v618 two exclusive wall-clock MRES packages

KINIT's `clk_minit` reads PCLK once, validates TIME36, and remembers the
choice in transient state. `pclkclock_minit` installs the PCLK MRES if
valid; otherwise `softclock_minit` installs the APR-backed software
calendar MRES. Both use the existing KCORE `wall_time36` JRST gateway,
and only the software provider redirects root-only SETTIME. The PCLK
hardware decoder is also compiled into transient KINIT for probing; its
resident copy exists only inside the selected PCLK package. KINIT
reclaims the uninstalled source package and probe after boot.

The APR scheduler/timeout clock remains one independent MRES. Its direct
`wall_clock_ticks` word stays in KCORE, preserving the existing interrupt
hot path and no additional runtime table. Software calendar arithmetic
remains outside PI context. Gregorian month days now use a bit mask
instead of a packed byte array, eliminating a GCC MRES boundary
relocation (POINT 9,label-1) and reducing stored data.

Lowmem linked v618 values: KCORE 8537 words, APR MRES 46, PCLK MRES 26,
software calendar MRES 329. Conservative KCORE+all MRES 15976;
PERMANENT_LAST 16023. Exactly one calendar package is installed at
boot; PCLK configuration omits 329 permanent words (15647 total),
while software configuration omits 26 (15950 total), relative to the
conservative linked report. This excludes no other optional MRES.
Native `daimos-clock-mres-v618` PASS with both `WALL_CLOCK_HZ=50` and
`60`, each in PCLK enabled/disabled boots. Native tests assert exactly one
PCLK or SCLK installation, UTC rollover and privilege checks. The
provider-specific resident totals above are computed from the confirmed
selection and linked module sizes.

## 2026-10-28 v620 eliminate unused APR calendar ticks under PCLK

The APR MRES exports the address of its `AOS wall_clock_ticks` instruction
as `CLK_X_WALL_TICK`. KINIT patches this word to `JFCL 0` (PDP-6 no-op)
when it selects PCLK, eliminating the second per-interrupt counter update
without a runtime conditional or another resident dispatch word. SCLK
retains the AOS unchanged. The shared scheduler tick and all device timing
are unaffected. Lowmem GCC KCORE+MRES remains 15976 words, unchanged from
v618; native `daimos-clock-mres-v618` PASS at 60 Hz in both PCLK modes.

A bulk-day/month software-calendar advancement candidate linked at 16018
words (+42), so it was rejected under RAM-first SIZEOPT criteria. The
original compact lazy per-second advancement algorithm remains intact.

## 2026-10-28 v621 single APR tick counter

The APR scheduler and optional software calendar used to maintain two
identical elapsed-APR counters. Both now use the existing fixed KCORE
`wall_clock_ticks` word, exported also as `clk_tick_count`. The APR MRES
increments it once; the optional SCLK MRES reads it directly. This removes
the duplicate hot-path AOS, the duplicate MRES BSS word, the separate
MRES tick-counter export and the v620 conditional instruction patch.
PCLK systems use the same scheduler counter but install no SCLK calendar.

The low-memory GCC link passes for 50 and 60 Hz: KCORE+all listed MRES
15974 words (v620: 15976), APR MRES 44 words (v620: 46). Native
`daimos-clock-mres-v618` PASS at 60 Hz and 50 Hz for both PCLK-on and
off boots. The first 50-Hz test run stopped with simulator SDL2 failure
(139); repeating with `SDL_VIDEODRIVER=dummy`, using an otherwise
identical kernel and test, passed. The initial failure was not a clock
assertion failure.

## 2026-10-28 v622 remove dead APR tick reader

The `clk_ticks()` reader had no callers; KINIT previously resolved and
immediately discarded its MRES export. Removing the unused two-instruction
reader and renumbering subsequent exports saves two resident words, while
the shared `clk_tick_count` / `wall_clock_ticks` word remains unchanged.
The lowmem GCC build reports APR MRES 42 words and conservative
KCORE+MRES 15972 words (v621: 15974). Native PCLK-on/off UTC acceptance
PASS at 60 Hz. The 50-Hz regression is tracked separately.
Native `daimos-clock-mres-v618` also PASS at 50 Hz, PCLK enabled/disabled,
with SDL dummy backend. Thus the two-rate v622 clock regression gate is met.

## 2026-10-28 v625 calendar leap optimization and build dependency

The lowmem boot Makefile's `KERNEL_INPUTS` omitted `drivers/wall_clock.c`.
This caused successive source-only clock experiments to leave the generated
assembly/MRES package stale and incorrectly report identical sizes. The
source is now explicitly tracked, so changes force rebuilt target assembly
and a fresh relocatable clock MRES link.

DAIMOS TIME36 supports only 2026..2125, where Gregorian leap years are
exactly `(year & 3) == 0 && year != 2100`. Replacing the general century
and 400-year divisions with that rule reduces linked SCLK MRES from 329 to
315 words (14 words saved). Lowmem GCC KCORE+MRES is now 15958 words;
KCORE remains 8537 words; the final permanent address decreases from
16019 to 16005. Native `daimos-clock-mres-v618` PASS at both 50 and 60 Hz,
PCLK enabled/disabled (SDL dummy backend). The change affects only the
software calendar and preserves the TIME36 year range and 2100 exception.

## 2026-10-28 v626 compact TIME36 century encoding

With fresh generated assembly after the v625 dependency fix, replacing
`year % 100U` in `encode_time` with one selection/subtraction based on
`year >= 2100U` saves three words in the software-calendar MRES: 315 to
312 words. Only years 2026..2125 reach this routine, so the arithmetic
is identical across the supported TIME36 interval. KCORE remains 8537
words; conservative KCORE+MRES is 15955 words and PERMANENT_LAST is
16002. The native clock test passes with PCLK enabled and disabled at
both 50 and 60 Hz (SDL dummy video backend).

## 2026-10-28 v630 early TIME36 validation checks

Move inexpensive hours/minutes/seconds range checks ahead of the
calendar-day validation in `decode_time`. All fields and bounds are still
checked, but the resulting target assembly has a shorter error path.
Fresh 60-Hz lowmem link: software clock MRES 312 -> 306 words, KCORE
8537 unchanged, conservative all-MRES 15955 -> 15949 words. Native clock
regressions PASS with PCLK enabled/disabled at 50 and 60 Hz.

## 2026-10-28 v631 BCD century arithmetic

Reorder the shared TIME36 decoder's year arithmetic to add the optional
100-year century adjustment to the base-2000 value. All BCD checks and the
2026..2125 bounds remain unchanged. This reduces the fresh lowmem-linked
SCLK MRES from 306 to 303 words; conservative KCORE+MRES becomes 15946
words, with KCORE still 8537. The native PCLK-enabled/disabled clock
regressions PASS at 50 and 60 Hz.

## 2026-10-28 v632 final SCLK optimization sweep

Inline the single-caller `encode_time()` function into `next_second()`.
The compiler eliminates the separate call/argument setup and reduces the
linked SCLK MRES by 11 words (303 -> 292). The permanent KCORE remains
8537 words, and conservative KCORE+MRES falls to 15935 words. Native
PCLK-enabled/disabled clock regressions PASS at both 50 and 60 Hz.

The four selected final topics are closed: (1) shared TIME36 decoder
validation ordering saved six words (v630); (2) alternative direct packed
calendar and repeated-calculation elimination grew the MRES (+8 to +22)
and were rejected; (3) single-caller encode inlining saved eleven words
(v632); (4) BCD century arithmetic saved three words (v631). No internal
clock representation redesign is planned. This closes the SCLK RAM-first
optimization sweep; further experiments require a new, measurable target.
