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

## Activation still pending

The FILE open/new-fd, vnode reconstruction, PIPE/FIFO remapping, duplicate,
close, lock-family handling, unmount cleanup and RUN descriptor inheritance
must be updated *together* to maintain a two-bit extension per descriptor.
There must be no truncation of high mount IDs and no uninitialized extension
bits on descriptor reuse. The existing mount limit remains **four**, so no
live operation yet relies on the extension word.

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
