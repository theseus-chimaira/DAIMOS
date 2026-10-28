# DAIMOS counter-only wall-clock migration (v614)

## Accepted interface and invariants

* KCORE maintains **unsigned elapsed seconds**, with APR ticks converted
  using installation-time `WALL_CLOCK_HZ` (50 or 60). No epoch, Gregorian
  arithmetic, MJD, leap years, or TIME36 encoding resides in KCORE.
* Userspace interprets the seconds as elapsed time since 1858-11-17 UTC.
  This interpretation is exclusively a userspace ABI convention.
* PCLK device 0730 is a **calendar** source, not a seconds counter. Its
  hardware DATAI/CONI must be read coherently in the privileged device
  layer, then made available as raw clock fields to an early userspace
  initializer. Userspace converts to seconds and calls privileged SETTIME.
  The kernel subsequently advances the seconds independently from APR.
* Do not convert PCLK's calendar fields into MJD/seconds in KCORE: that
  would defeat the kernel/calendar separation. A raw-PCLK read ABI and
  the boot-time initialization sequence must be introduced together.
* GETTIME, filesystem mtimes, UTIME, `time()` and DATE must all migrate
  **atomically** from packed TIME36 to unsigned seconds. Existing disk
  timestamps require versioned migration or explicit compatibility, not
  reinterpretation. Zero is currently the clock-unset sentinel; the
  all-zero epoch instant is consequently unavailable unless a distinct
  validity indicator is introduced.
* Do not delete old TIME36 fallback until the boot path, native PCLK-on
  and PCLK-off testing, filesystems, and all date consumers pass against
  one consistent seconds ABI. Avoid simultaneous incompatible storage.

## Completed preliminary work

The independent userspace `time1858_encode` / `time1858_decode` routines
build with PDP-6 KCC. `daimos-time1858-v614` C99 testkit PASS for epoch
zero, Unix epoch offset (3506716800 seconds), Gregorian leap-day rollover
and invalid leap day. **Kernel seconds ABI and PCLK raw-read API remain
unimplemented**, so the installed kernel still returns TIME36.
