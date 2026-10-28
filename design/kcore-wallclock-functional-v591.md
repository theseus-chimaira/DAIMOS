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
