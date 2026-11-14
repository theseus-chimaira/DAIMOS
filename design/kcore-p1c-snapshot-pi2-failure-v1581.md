# KCORE P1-C v1581: physical snapshot-header PI2 fail-stop

Date: 2026-11-14. Scoped physical direct-DEVICE BACKSTORE failure
acceptance. P0-A and trusted-operator P0-B remain closed; P1-C
remains OPEN for independent swap-in overlap and mapped VOLUME
errors. VERSION 0.5, strict 32K/optimization HOLD.

## Source-authentic physical path

This increment builds the original full 128K KCC PDP-6 kernel with
a disposable diagnostic source copy. It reuses v1580's successful
physical workload: DSK270 D6FS root, actual Type-236 AUXSTORE,
persistent MEMFS data/raw-XIP, genuine LOGSTORE append/readback,
and four large independent sleeping private-image processes.
The original privileged HALT initiates `fs_memfs_shutdown` while
actual KCORE `proc_swap_blocks_used` is positive.

A test-only selector marks precisely the first one-sector write to
the final BACKSTORE block whose source header starts invalid (zero).
The real `drm236_request` binds the resulting physical WRITE
descriptor on the caller's private stack. On the genuine Type-236
DR_DONE PI2 event, the optional test selector routes only that
active descriptor through the original `drm236_pi_error` path.
The original driver still publishes the negative completion and
wakes the owner; the original v1541 BACKSTORE gateway then dispatches
to the original v1515 critical-device reason 2 HALT. No I/O result
is manufactured at the BACKSTORE gateway, and the original hardware
transfer must happen for the error to be selected.

Actual physical result: the linked critical leaf begins at `004002`
octal, and the independent real PDP-6 simulator halted at the unique
BACKSTORE instruction `004011` octal. Diagnostic post-HALT KCORE
words show PI2 selected-error count `1`, the original DRM driver's
`mfsdev_storage_errors+5` count `1`, actual owned swap blocks
`01374` octal, active BACKSTORE enabled `1`, selected last logical
sector `07777`, physical Type-236 hardware group `0157670`, and
retired descriptor and trigger both zero. The raw Type-236 drum
contains an invalid snapshot header at physical block `015767`,
with 248 nonzero lower BACKSTORE sectors. Separate AUXSTORE regions
and the descriptor remain unchanged. The test observed an actual
DR_DONE event, not arbitrary injected failure before device I/O;
it does not claim natural magnetic-media damage.

Seven deliberately corrupted postflight evidence sets are rejected:
no PI2 error, missing original driver error accounting, no swap
ownership, wrong hardware group, descriptor
left active, wrong HALT PC or a falsely committed physical header.
All work and the five diagnostic fixed words exist only in the
testkit's disposable build. Original DAIMOS has no code, RAM,
ABI, filesystem format or utility change. Original KCORE low end
remains `024364` octal; the disposable diagnostic build has its
own different map and is not a production size measurement.

## Remaining gate

This is direct DEVICE storage. It does not establish a mapped
multi-member VOLUME controller failure, independent PI-driven
process swap-in completion *during* synchronous snapshot transfers,
or natural hardware fault behavior. Nor is the single fatal HALT
a substitute for checking a healthy provider's independent
BCACHE/MM/process rollback under recoverable errors. Those are
remaining P1-C cases. P1-D alternate physical inventories and
P1-E uncompressed PURE integration remain separate open tasks.

Reproduction and strict full-byte evidence are in
`daimos-testkit/tests/validation/daimos-p1c-snapshot-pi2-physical-v1581.sh`
and companion `.py` injector, `.py` verifier and `.md` test report.
