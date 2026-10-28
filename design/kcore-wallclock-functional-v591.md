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

## 2026-10-28 v594 unified TIME36 gateway

Introduced `wall_time36` as the single entry for GETTIME and D6FS/MEMFS
timestamps, including the provider C path. The original `pclk_time36`
continues to read physical PCLK and the gateway currently tail-jumps to it.
This preserves timestamp semantics while allowing a future fallback
calendar in one place. No software fallback or SETTIME is enabled yet.
