# Proposal B v401: canonical descriptor metadata audit

Status: design-only. No production layout migration is authorized.

## Confirmed source dependencies

- `fs/file.h`: 16 two-word descriptors. Word zero currently packs a vnode
  fragment with READ, WRITE, DIR/APPEND, two-bit regular/lock state and a
  four-bit lock-family token. Word one is an independent 36-bit offset.
- `fs/file_runtime.s`: `file_node_from_fd`, `file_mount_ext_store`,
  `file_mount_ext_copy`, `file_lock_mount_ext_v319`, and the inline mount
  extraction in `file_unlock_mount` use u-area word 067. OPEN, FIFO
  substitution, CLOSE, DUP/DUP2, READ/WRITE, READDIR, SEEK and FLOCK depend
  on descriptor metadata or vnode reconstruction. Direct-device dispatch
  compares complete canonical endpoint vnodes.
- `proc/proc_run.s`: mapped descriptors copy both words, independently
  copying offsets, plus the corresponding mount-extension bits. RUN also
  transfers credential/PURE state and increments pipe references.
- `proc/proc_pdp6.s`, `proc/syscall_dispatch_pdp6.s` and
  `boot/kinit_late.c` also consume or construct the packed format.
- `fs/file.c` contains an alternative C OPEN/FIFO replacement path.
- `proc/proc_swap.h` and `proc/proc_pure_backing_pdp6.s` reserve credential
  word 066 bit 27 for the fifth PURE mount-slot bit. Treat it as LIVE.
- `boot/kinit.c` allocates six MM-backed mount arrays using
  `root_select_mount_capacity_v22()` and publishes `vfs_mount_limit` last.
  `fs/vfs.h` retains an outdated `VFS_NMOUNT=4` comment/constant despite
  this runtime capacity path; audit actual remaining users before changing it.

## Storage feasibility

An uncompromised canonical vnode plus 36-bit offset occupies all 72 bits
of the current descriptor. Retaining existing semantics with direct fields
requires up to nine metadata bits per fd: READ 1, WRITE 1, DIR/APPEND 1,
regular/shared/exclusive state 2, and family token 4. Sixteen descriptors
therefore demand 144 bits or four 36-bit metadata words. The current mount
extension uses one 36-bit process word, so a naive replacement costs three
more process words (before alignment, code or stack costs). This is a
storage bound for the current field model, NOT a proof all nine bits must
survive a coordinated redesign.

No no-growth B1 encoding is yet demonstrated. Do not assume high offset
bits are spare, or overwrite credential/PURE state. The next investigation
should establish whether OPEN permissions and APPEND can be encoded more
compactly, and whether equivalent FLOCK ownership can be represented using
existing state without new process words or provider lookups on hot paths.

## Migration and acceptance plan

1. Produce an exhaustive per-field reader/writer table including abnormal
   close, lock owner equality, DUP2 replacement, inherited pipes and process
   teardown. Identify semantic invariants versus redesignable machinery.
2. Compare B0 (packed), B1a (canonical plus four metadata words), and any
   provably correct compressed B1b. Count u-area words, extra MM allocations,
   resident text/data, code to access fields, and per-operation calls.
3. Verify boot-calculated mount capacity, provider arrays and PURE owner
   lifetime at mount ids 16 and 26. Do not claim the 26-mount planner as
   demonstrated 26-volume live boot coverage.
4. Add migration tests ONLY in daimos-testkit; ensure descriptor-format
   producers and consumers move atomically. Exercise two identical local
   vnodes on distinct mounts, locks, close rollback, FULL 36-bit seek,
   APPEND, DUP/DUP2/RUN independent offsets, FIFO/PIPE refs, mount lifetime,
   swap/PURE and disk/tape boot.
5. Upon explicit design approval, evaluate linked PDP-6 and PDP-10 images,
   total physical RAM, per-process RAM and runtime paths. Revert negative
   experiments. The strict 32K gate and JRST folding remain deferred.

## Measurement baseline (not remeasured in v401)

v388: KCORE 9109 words; KCORE+MRES 15657; PERMANENT_LAST 15704.
To reach strictly below 8192 KCORE words requires at least 918 saved words.
The v19 historical +138-word mount expansion is not a v401 saving estimate.
