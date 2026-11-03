# Final independent KCORE tip audit (2026-11-03, v1065)

Scope: consolidate **history**, not resume KCORE optimization or run
deferred 32K acceptance. The last divergent DAIMOS tips were:

* `kcore-mres-selective-integration-20261023-v1`, tip `b2f92fd8`.
* `kcore-resident-startup-snapshot-v72-20261020`, tip `bafe9fd8`.

The corresponding testkit tip was
`test-daimos-startup-snapshot-v72-20261020`, tip `68b9169`.
The original commits are preserved as main ancestors through
history-only merges (`-s ours`), not code integration.

## Selective MRES tip: classification

* Type-270 disk and Type-167/236 drum address wrappers moved into
  optional MRES in the old `dee93223` proposal. The matched v1038
  current-source experiment showed KCORE -31 decimal words but DSK
  +18 and DRM +14, so **one extra permanent word** with both present.
  The extraction was rejected on RAM-first grounds; its original
  patch remains recoverable and might help configurations without
  both devices, but it is not current production.
* The VFS parent-register fix was separately recovered (v1037) and
  passed the negative-control/real AC3 native PDP-6 regression,
  saving one linked word at that baseline. The check was rerun and
  passed during this consolidation.
* The historical DSH counted-SIXBIT compression and full-object
  folding are represented by later accepted current DSH work. Do not
  restore the obsolete userland make or older size limit.
* The `5ab3c4d1` syscall service-tail folding remains a historical
  unaccepted ABI-dependent size candidate. Do not replay it into the
  newer dispatcher without explicitly resuming KCORE optimization.
* The branch's KCC MonitorFS reader selection is superseded by the
  smaller **GCC** cold reader: v1033 compared 003070 vs 003265
  octal MRES words (GCC vs KCC). The v1033 test was rerun with both
  KCC and GCC kernel selections and passed. This is a dated compiler
  selection result, not a new size measurement.

## Resident startup/snapshot tip: classification

* The v72 `proc_startup_snapshot_v72.c` path retained validated,
  dynamically allocated argv/envp records for swapped processes.
  Current DAIMOS does not contain that helper or its u-area ABI.
  It must not be transplanted into current swap/MonitorFS readers
  without ownership, size, and lifetime revalidation.
* The v98-v99 optional SYSINFO MRES and v94-v97 syscall-tail folding
  depend on earlier dispatch and module ABI layouts. Current main
  keeps PROCINFO/MEMINFO in its existing syscall implementation;
  there is no `SYSINFO_MRES_ENABLED` boot switch. The old tests are
  historical rather than live regression gates.
* The v102 backing-allocator relocation reported **98 fewer KCORE
  words** in its historical profile, using a separate pinned BALOC
  MRES. Current main instead has the later v484 KCORE entry and SWAP
  MRES backing allocator; the original layout is obsolete. No v102
  saving is added to current figures, and mandatory resident MRES
  overhead must be included before any RAM benefit is claimed.

## Actually recovered test and build work

* The historical v101 swap-source-graph test found four missing
  outer boot Makefile inputs of the generated native swap object.
  DAIMOS v1063 added them, and testkit v1063 passes; the unfixed
  source fails the same check as a negative control. No code size
  or KCORE text was changed by this build dependency repair.
* The historical FILE target allocated space for the mount-extension
  word at `file_table+041` octal. Testkit v1064 restored and zeroed
  two extra fixture records, removing a potential test stack overrun.
  The direct native FILE target test passes without compiler warnings.
* The v72 startup-snapshot target, old 48K MonitorFS stress, optional
  SYSINFO tests, and old FILE jump-slot test retain assumptions no
  longer supported by current main. Their exact sources remain in
  historical ancestry, not active default test targets. See testkit
  `tests/archive/startup-snapshot-history-v1064/README-v1064.md`.

## Verification and open work

Passed on current source: VFS parent AC3 v1037 native PDP-6;
MonitorFS compiler selection v1033 under both kernel compilers;
the repaired FILE target; and the v1063 swap dependency source test.
No 32K-specific tests, new KCORE optimization, or full system boot
were run for this last-tip consolidation. The historical syscall
tail folding, startup snapshot, 48K stress, and size experiments
remain available by their original commit IDs if explicitly resumed.

After verifying the pushed main merge ancestry, the three old local
branch references can be deleted without losing any commit content.
