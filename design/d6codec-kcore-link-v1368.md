# v1368: grouped codec KCORE/MRES linked RAM and low-core layout

Date: 2026-11-08. Status: **full temporary PDP-6 KCORE/MRES links**,
not a production codec switch or operating-system cold boot.

## Provenance and nonproduction scope

The native grouped v1367 codec and VFS adapter already passed
bounded streaming, trusted backing, malformed group and all current
128-word KCORE/MRES-prefix regression windows. v1368 measures the
resident impact on an actual fully linked KCORE rather than using
only the 317-word standalone codec total.

`daimos-testkit/tests/validation/d6codec-kcore-link-v1368` obtains
the **real current KCORE object list** and the existing p10fold
donor scope by evaluating the build Makefile. It uses only temporary
files under `$TMPDIR`. Re-linking the production KCORE with its
current fold plan yields a **byte-identical DXR**, RIM and initialized
boot stream. This establishes a reliable unchanged control.

The test assembles a private v1367 VFS frontend with symbol aliases
`d6lz36_decode_vfs` and `d6lz36_decode_buffer` so all preexisting
object references bind exactly as they do today. The grouped v1367
stream object is linked next to it. No production source or artifact
is edited and no old-format stream is passed to the grouped frontend
for execution.

## Actual linked KCORE sizes

| Link configuration | Text | BSS | Text delta | KCORE exclusive end |
|---|---:|---:|---:|---|
| Production D6LZ36 | 9601 | 269 | 0 | `023276` |
| Grouped, no old low-core slot | 9817 | 269 | **+216** | `023626` |
| Grouped, 40-word non-executable reservation | 9857 | 269 | **+256** | `023676` |
| Grouped plus legacy low-core decoder (measurement only) | 9857 | 269 | +256 | `023676` |

The table uses the regenerated fold plan. Keeping the production
fold plan instead gives text deltas +220 and +260. `p10fold -m 2`
selects 31 folds/75 donor words after adding the two grouped
objects, compared with 29 folds/71 donor words in the original
plan. Two additional two-word folds involving grouped code save
four words. Folding therefore does **not** solve the remaining
resident code growth.

The minimum grouped candidate shifts `exec_replace_current_lowmem`
from `000407` to `000337` because the 40-word fixed D6LZ36 image is
gone. The reserved variant retains its exact original `000407`
address without retaining a second executable codec. A real KINIT
and Stage-1 installation sequence must establish that the former
low-core slot can be reclaimed safely before accepting the smaller
candidate. KCORE still links at `000060` in both variants.

## MRES ABI relocation and boot stream conversion

Five real linked relocatable MRES packages are rebuilt against
the reserved and no-slot grouped KCORE maps. The original-map
relinks reproduce the production MRES files byte-for-byte.
All grouped-map relocations resolve with identical MRES residency:

| MRES package | Unchanged resident words |
|---|---:|
| SWAP | 1757 |
| DTFS/TSFS | 1834 |
| D6FS | 2175 |
| MonitorFS | 1592 |
| Tape | 405 |

The test also converts both grouped KCORE DXRs into SIMH RIM and
initialized-word streams using their new linked
`__kcore_low_init_end` values; the baseline conversions reproduce
the existing production files byte-for-byte. The no-slot initialized
end is `023211`; the reserved-slot end is `023261`. These are valid
**test artifacts**, not installed KINIT package or actual boots.

## RAM decision and next gates

The 216-word minimum text increase remains material in a 9,601-word
KCORE and misses the project's below-`020000` octal size target by
a wide margin. Keeping absolute early EXEC addresses pushes the
increase to 256 words. KCORE BSS does not grow. The measured five
MRES packages do not contribute further residency growth.

Source inspection confirms `vfs_read_words` routes real non-PIPE,
non-MonitorFS requests through `fs_provider_reg_call`, which may
retain a seven-word serialized-provider frame while the provider
waits or performs physical I/O. The 67-word v1367 VFS frame and
the leaf fake-provider stack canary do NOT bound that full nested
call chain. Native cold read with stack watermarking is mandatory.

Before a production cutover: choose and validate the low-core KINIT
installation layout, continue RAM reductions, measure full nested
FS/EXEC/PURE/TSFS stack usage and partial-failure cleanup, test
full runtime grouped media, and atomically migrate all host
producers, kernel consumers, Stage-1 and filesystem images. No
permanent mixed-format fallback or duplicate resident decoder.
