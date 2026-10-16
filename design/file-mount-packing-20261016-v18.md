# Two-word FILE descriptor mount expansion — credential repacking v18

## Goal and format constraint

26 mounts need a five-bit zero-based mount slot. A canonical vnode's six-bit
mount field currently contributes only three bits to packed two-word FILE
records; the other three bits carry FILE metadata. Extending to five mount
bits therefore needs two additional bits **per open descriptor**, while
preserving the complete 36-bit file offset and all existing FILE metadata.

With 16 descriptors, the extra two-bit values need 32 bits per process.
Expanding each descriptor to three words would waste 16 words per process.
Adding one u-area word would enlarge every process even when no file is open.

## Existing-word solution

The stable per-process u-area already contains two words after its 16 FILE
records: credential word `066` and former umask word `067`. UID and GID are
only 9 bits each, and umask is also 9 bits. Repack them as follows:

| u-area offset | Bit positions (LSB first) | Meaning |
|---|---|---|
| `066` | 0..8 | GID9 |
| `066` | 9..17 | UMASK9 |
| `066` | 18..26 | UID9 |
| `066` | 27..35 | Reserved; one candidate bit for PURE mount extension |
| `067` | 0..31 | 16 × 2-bit FILE mount extension map |
| `067` | 32..35 | Reserved |

The v18 source change implements the **credential/umask packing** now.
`PROC_UMASK` reads bits 9..17 of word 066, `proc_control_umask` modifies
only that field, and SETUID/SETGID update only their respective 9-bit fields.
All credential readers tested for root privilege, UID/GID and file ownership
discard upper halfword bits. RUN inherits the entire packed credential word.
The u-area remains `0410` octal words, the FILE record remains two words,
and word `067` is reserved for the forthcoming descriptor mount extension.

## v19: native FILE mount-ID extension implemented (2026-10-16)

The native descriptor format now stores mount bits **3 and 4 of the mounted
vnode's six-bit mount field** in u-area word `067`, with each fd using two
bits at position `2*fd`. These bits are vnode positions 27..28 and formerly
collided with the lock-family metadata, **not** the local-kind state bits.
`file_new_fd` validates the provider/local-kind spare bits and rejects
mount bit 5, saves the extension bits before overwriting lock-family bits,
then stores the unchanged two-word FILE record. `file_node_from_fd` restores
the extension whenever a canonical vnode crosses into VFS or syscall/TTY
dispatch. Descriptor duplication, DUP2 overwrite, FIFO substitution, close,
unmount lock cleanup and FLOCK comparisons preserve or consult the map.
`proc_run_map_loop` transfers each mapped fd's two bits into the zeroed child
u-area at the new fd position. The alternate C `file_open` path keeps the
same extension updates when replacing an opened FIFO with its pipe vnode.

Provider 7 is shared by TSFS and PIPE. Checking only the provider or the
three packed low mount bits can misidentify mounted TSFS roots as pipes.
FILE close/dup and native read/write syscall fast paths therefore verify the
reconstructed full vnode. `pipe_add_refs` consults the **child's** extension
map during RUN inheritance instead of assuming the current parent's
`file_table` owns the child. Both native pipe-reference and TSFS-close
regressions pass.

The existing full simulator `daimos-pipe-fifo-v2.sh` and
`daimos-pipe-wordfifo-v2.sh` regressions also pass with the documented
**diagnostic-only** DSH image limit override (`037000`); the default `036000`
DSH image-size gate remains unchanged and blocks these test builds without
that override. This is not production DSH size acceptance.

PDP-6 tests in daimos-testkit exercise two otherwise identical vnodes on
mount IDs 2 and 26, lock independence, DUP/DUP2, close cleanup, unmount
cleanup, TSFS-root close-vs-PIPE dispatch, RUN descriptor remapping, and
PIPE references with a TSFS root on mount 8. The diagnostic disk-root SIMH
boot reaches ROOT login and ECHO under the existing test-only DSH limit
override. Dedicated disk/tape permanent-size gates pass.

There is no extra per-process RAM and the FILE descriptor offset remains
36 bits. The additional native addressing, lifecycle updates and guard
paths cost **138 permanent words** versus v18 (disk single last `037171`,
multi last `037232`, tape last `036353`, octal). GCC 32K lowmem KCORE ends
at `024006`, exceeding the strict `020000` gate by 2054 decimal words.
The disk/tape KINIT images end `072706`/`073645`, leaving 1594/1115 words
after stack reservation. These measurements need no shell and do not certify
32K runtime boot.

## Activation still pending after v19

FILE mount extension consumers are now converted and verified in native
execution tests, but **full 26-mount runtime activation is not complete**.
The separate PURE swap backing format, optional DTFS personality table and
remaining provider-specific bounds must be converted before KINIT increases
the active `vfs_mount_limit` above four. The 32K KCORE gate still fails.

PURE swap backing independently needs five slot bits but currently needs
18 + 4 + 14 bits for vnode index, mount slot and text length; growing the
slot to five creates a 37-bit record. A **potential zero-word solution** is
storing its single additional mount-slot bit in one of the unused high nine
credential bits of word 066. The u-area remains resident while a process is
swapped and each process has only one PURE backing record. This is not yet
implemented: attachment, restore, busy checks, release, EXEC and RUN must
all preserve or explicitly initialize the bit before this is safe.

## Native validation

`daimos-testkit/tests/validation/daimos-file-credential-bits-v18.sh`
executes the exact PDP-6 credential/umask assembly leaves and checks that
nonzero spare UID-half bits do not confer root, nonroot SETGID still compares
GID9 only, invalid GID changes are rejected, SETUID/SETGID preserve UMASK9,
and UMASK updates do not touch word `067`. The standard u-area layout test
confirms that the overall u-area size and FILE offsets have not changed.

This precursor changes linked permanent KCORE by +9 words from v17:
disk-root last `036757` and tape-root last `036141` (octal). GCC lowmem
32K KCORE ends at `023574`, failing its strict `020000` gate by 1916 decimal
words. Disk/tape KINIT ends are `072474`/`073433`, with 1732/1253 words
free after the reserved boot stack. These are build/layout measurements,
not 32K boot acceptance, and require no shell.
