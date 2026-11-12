# KCORE BACKSTORE: physical Type-167/236 PI2 read-error propagation v1511

Date: 2026-11-12. Scoped physical acceptance of the real drum
controller-error completion and its original swap-in failure/rollback
path. Production KCORE remains the accepted v1507 version; no
new process state, resident allocation, ABI change, forced resource
release, or scheduler shortcut is introduced.

## Actual source path and experimental boundary

The previous v1508-v1510 physical tests refused a checked
BACKSTORE *free* after a successful payload read. They did not
exercise Type-167/236 PI2 error completion. The first attempt to
instrument v1511 at the Type-270 DSK DEVICE was rejected because
the actual physical BACKSTORE used the Type-236 DRM controller.

The corrected testkit v1511 copied the real F07 256K KCC PDP-6
guest and installed a disposable observer at the original
`backstore_read` gateway, the live `drm236_request` private-stack
descriptor, and the Type-236 `DR_DONE` PI2 handler. The original
bad swap slot was chosen only after a real payload transfer, and
its original bitmap-owned run remained intact. For the next two
genuine reads of the same owner, the corresponding live DRM
descriptor was redirected to the **existing**
`drm236_pi_error`/`drm236_finish_active_v1079` code. Its real
error-accounting and event-wakeup path reached
`backing_region_io`, `proc_swap_in_read_fail` and the ordinary
MM unpin/free rollback, leaving the owner DISK-backed and its
deferred TERM pending. After those two failures, normal hardware
read completion resumed and the original checked-free refusal
continued. Reads of other processes remained unaffected.

Source-link evidence directly samples the selected DRM PI2 error
counter, actual `mfsdev_storage_errors+5` controller error word,
exact DISK owner and failed swap-service returns, plus the
already accepted resident PID 5 and swapped PID `011` user
syscall/restore witnesses. The physical read error is a genuine
DRM driver negative completion **injected at a real hardware
completion**; this is not a damaged magnetic medium or a missing
controller with no PI2 interrupt.

## Physically verified result

The complete 256K KCC PDP-6 diagnostic produced 36 source-linked
read-only SIMH snapshots, with thirteen actual fault-on snapshots
24..36. The strict v1511 physical verifier PASSED and the original
selected DISK owner was exactly slot `010`, first BACKSTORE block
`02350`, length `0330` octal; the authoritative original record
was `002350000330` at all fault-on samples. Its stable u-area,
nonresident VM, v1507 retry marker, bitmap/accounting and pending
TERM survived all error/checked-free rollback attempts. No false
READY_ONCE, owner-side TERM consumption, fatal close, finalizer,
zombie, or bitmap repair occurred.

| Physical sample | PI6 clock (decimal) | DRM PI2 errors | Checked free refusals | PID 5 SLEEP | Swapped PID 011 SLEEP |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 24 | 36238 | 1 | 1 | 58 | 0 |
| 25 | 37753 | 2 | 3 | 162 | 0 |
| 26 | 39278 | 2 | 5 | 239 | 0 |
| 27 | 40795 | 2 | 7 | 305 | 0 |
| 28 | 42313 | 2 | 9 | 371 | 0 |
| 29 | 43832 | 2 | 11 | 437 | 0 |
| 30 | 45351 | 2 | 13 | 484 | 0 |
| 31 | 46869 | 2 | 14 | 518 | 34 |
| 32 | 48388 | 2 | 16 | 584 | 100 |
| 33 | 49906 | 2 | 19 | 674 | 190 |
| 34 | 51427 | 2 | 21 | 749 | 265 |
| 35 | 52944 | 2 | 23 | 815 | 331 |
| 36 | 54466 | 2 | 25 | 881 | 397 |

At the end the controller had accounted **two** actual PI2 error
completions, the selected BACKSTORE free refused **25** times,
and real slot-0 failure accounting was exactly **27**, not a
misleading count of all 27 as checked free errors. Throughout
18,228 PI6 ticks, independent resident PID 5 progressed from
58 to 881 actual SLEEP entries (+823). Independent swapped PID
`011` was nonresident during the fault, committed a real checked
ordinary restore before sample 31, and executed **397** genuine
SLEEP syscalls by sample 36. Its alternate exchange witness
remained zero.

Both source-linked negative tests and physical evidence mutation
tests pass: five malformed DRM/gateway source variants and eleven
unsafe ownership/accounting/progress variants were rejected.
Native source-linked DRM v1079 watchdog, v1080 FIFO and v1247 PI
AC3..AC6 preservation also PASS. The diagnostic guest was
intentionally stopped after samples, **not a normal HALT** or
terminal resource cleanup; the original DISK owner remains held.

## Evidence fingerprints

Guest: `$TMPDIR/daimos-oom-requester-live-v1075.3YVnE1`.
Runner: `$TMPDIR/daimos-swap-no-partner-v1304.vjhHP1`.

```
addresses          1cb94c0d0d98abdcbafff9355450c726c76f77a5ae02cfc06177e98fdfe86705
simh-sampler.log   2f8189e1fdb9bdc97789c1cb7a870b0ba68a8006397bf02a76e6665a5342a77d
simh.out           771f1422a9250053b4422bcf4fadf51c8ba00171cc3a788d8e7151706c3f73dd
build.log          86b410c00089a0398a4ca6d031607d957faf2382d4beb1a9b1455907aa7577db
kcore.map          761b847e05fbd35f64fa90781dbe483d80d455c6522842688f1f953478a8ba58
```

Testkit preparation and exact controller revisions include
`1a5c443`, `d46872d`, `37e6f8c`; finalized physical evidence
`25df8fa`. The earlier Type-270 error-target experiment was
rejected and is not counted as an accepted KCORE test.

## Accepted boundary and next work

Accepted: **real PI2 Type-236 drum error completion** on the
original failed BACKSTORE owner, correct runtime negative request
and MM rollback, safe retained allocation and independent healthy
resident and swapped process progress. The test does *not* prove
failure of real magnetic hardware, lost PI2 completion, actual
media reconstruction, permanent-device-offline policy, or safe
release of the original irrecoverably corrupted private owner.
The versioned testkit v1512 separately prepares a real PI6
300-tick lost-completion watchdog physical diagnostic; only its
native and runner-preparation checks are accepted so far.
Mixed DEVICE/VOLUME, BCACHE/DTFS, LOGSTORE/MEMFS, PURE/MM/APR,
and dynamic FSINFO feature gates remain open. Compression/size
optimization is deferred until these features are complete.
