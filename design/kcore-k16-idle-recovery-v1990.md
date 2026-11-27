# K16 v1990: directory FULL recovery in the original idle service

Parent 390ac172. Native and linked-package acceptance, not yet
physical FULL fault replay. New opt-in DAIMOS_PBM_IDLE_RECOVER_V1990
requires DAIMOS_PBM_DIRECTORY_V1983=1 and inherits strict swapless
TSFS/no-v1667 constraints. Both default OFF. No fixed scheduler,
trampoline, reaper, or retry-clock instruction/data change.

## Retry authority and dispatch

The existing reaper scans TRANSITION owners without FINALIZE or
PREVM and dispatches bit32 (swap retry) through tagged command
0400000|slot to the installed PURE service. It does not interpret
directory phases. In v1990 only PAIR/FREE/SOURCE set bit32; HOLD
retains both kind bits, source sector, exact length and heap/target
escrow but clears bit32. This is necessary even when HOLD did not
itself arm the shared clock, because another owner can trigger a
scan. All changes to the phase and its retry bit are PI excluded.
The standalone v1989 format is unchanged with v1990 OFF.

Original slot-zero mode3 preflight is shared by arming and retry.
Only the initial synchronous failed FULL after clean-unlock refusal
may arm. It validates phase, original exact geometry, original
heap encoding, stable u-area/backing, and exact saved MM owner/pin
set. Ambiguity at the MM witness revokes authority to HOLD.
The original armer temporarily replaces word067; restore exact
escrow while PI is excluded. No new clock word or queue.
Tagged service routes mode3 to the directory rollback and every
other mode to the original RAW handler. It does not alter the
scheduler cursor or advertise READY. A successful rollback
unlocks inactive mode3; later normal FULL may load and dispatch.
A deferred refusal must not call the first-failure armer again.
The original idle scan advances backoff before retry, keeps pending
on failure, and retires the shared pending state after all owners
succeed. Changed exact MM geometry changes phase to HOLD; the
current scan records refusal, next due scan skips it. Malformed
preflight that cannot establish a trusted transaction remains
untouched and may keep retry polling; no MM mutation is authorized.
Unknown/source-loss ownership and hard faults remain quarantined.

## Validation

Testkit09d4935. Actual v1989 relocated FULL/rollback, real MM,
codec, loader, complete PURE service, RAW PREFIX, original idle
deadline and epoch update, armer, retry scan and capped backoff
linked natively. Outer idle loop/interrupt handoff and terminal/
discard callbacks controlled. Each GCC/KCC PDP6/KA10 82 FULL,
80 explicit recovery and127 service/deadline calls. Exact heap
and length round-trip host oracle16,777,216 cases with new bit
convention. Three known rollback refusal phases, 17-bit wrap,
one-tick-early refusals, delay16->32->64->128->256->512 and cap.
Recovery without mount, exact target retirement and pin release,
no READY or cursor change, later normal restore. HOLD skipped
during unrelated scan; changed descriptor revokes authority even
if later repaired. Malformed command, nonzero current slot refuse.
Independent exact RAW owner and directory owner in same scan;
RAW unlock preserves pending directory retry.

Eight executed unsafe variants per compiler reject: no-arm,
no-route, clobber-cursor, raw-to-bitmap, rearm-every-retry,
early-retry, no-backoff, scan-hold. Original standalone v1989
OFF regression KCC both CPUs and all9 mutants PASS.

Initial fixture signature mismatch corrected using real JRST
gateway ABI and unsigned int terminal status. Initial combined
fixture exceeded reserved arena boundary at040000 because unused
historical mains linked; stopped simulator and omitted unused
entry points only. Final runs PASS. KCC unused fixture helper
notes and existing GCC include-order warning; no production
toolchain defect demonstrated.

Source-built MRES matrix both compilers OFF/ON/OFF/ON in same
build directory. DOBJ and map reproducible; new symbols only in
enabled MRES, fixed KCORE DXR byte-identical across modes.
GCC5460->5917 resident (+457), KCC6504->7055 (+551); BSS0,
no extra fixed/per-process storage. Both actual Makefiles reject
recovery without directory or nonboolean recovery flag.
This is linked MRES measurement, not full physical stack/latency
measurement. Testkit Makefile targets reproduce native/package tests.

Evidence relative TMPDIR:
pure-bitmap-placement-v1985.SfnNWa KCC,
pure-bitmap-placement-v1985.WVtXrs GCC,
pure-bitmap-placement-v1985.lrRh7s original v1989 OFF,
pure-directory-idle-package-v1990.6BCnib package matrix.

## Next K16 gate

Source-build full bootable v1990 image using exact v1987 guest
bytes; physical controlled FULL fault and actual idle retry.
Use precise test-owned fault authority (e.g. original known
target cleanup refusal), actual clock/deadline and exact saved
owner/pin/heap witness, then release only the introduced refusal.
Require no synthetic scheduler or retry state, actual original
mode3 rollback and later restore/EXEC/WAIT, final MM/HALT.
Recalculate matched capacity with increased MRES, do not assume
v1985 twelve-child result persists. Dirty DROP, irrecoverable
source loss/corruption, real DSH/48K, latency/stack still open.
Default OFF.
