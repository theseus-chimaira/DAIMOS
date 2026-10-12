# DSH lazy history storage and shared word allocator — 2026-10-12 v10

This stage follows the verified v9 compact/lazy AST work. No DSH language
feature, history capacity, argument limit, saved-function limit, or kernel
interface has changed.

## Changes

The history cache is still exactly 16 entries of 31 PDP-6 words each
(496 words maximum). Previously that storage was reserved permanently
in BSS for every DSH instance, including noninteractive shells that never
use history. The first valid history record now reserves the full cache
via BRK. History startup load happens before command parsing; interactive
command insertion happens before evaluation. Consequently the allocation
does not invalidate the parser's temporary BRK checkpoints.

The history insertion loop now shifts only the existing live entries;
the first record does not read or copy the other fifteen uninitialized
heap entries. Once the cache reaches sixteen entries, insertion still
discards the oldest entry in the original order.

Duplicate bounded BRK-reservation implementations in history, AST parse,
stored functions and large-argv handling were merged into one
`dsh_brk_reserve_words` helper. The helper validates requested word
count, current break, address-space bounds, and successful extension.
Each caller retains its own lifetime and release policy.

## Measured PDP-6 sizes (octal words)

| Metric | v9 | v10 |
| --- | ---: | ---: |
| DXR image | 035671 | **035673** |
| Static BSS | 005013 | **004034** |
| Image+BSS+02020 startup/stack | 044724 | **043747** |
| Rounded initial VM | 046000 | **044000** |
| Static KCORE end | 023610 | 023610 |
| KCORE+initial VM lower bound | 071610 | **067610** |

The BSS saving is 495 decimal 36-bit words; the initial aligned user
allocation drops another 02000 octal words (1024 decimal). On first
interactive history use, 496 heap words may subsequently be committed.
This distinction matters: the improvement is **initial process memory**,
not a claim that 16 fully populated history entries require fewer bits.

## Acceptance

- Fresh full PDP-6 boot-image build with the normal 036000 image limit: PASS.
- Complete DSH CTY target conformance: **115/115 PASS**.
- Focused AST/function/20-word FOR and nested SOURCE: **16/16 PASS**.
- Two-boot interrupted S6REC HISTFILE repair, valid-prefix recall,
  readback and clean D6FSCK: **5/5 PASS**. The testkit's v10 script
  also corrects checker linkage after `u_s6_pack` moved into
  `libc/u_pack.c`.
- Versioned DXR/BSS/VM size regression guard: PASS.
- Dedicated 32K LOWMEM INIT/LOGIN/DSH: **FAIL**, ROOT still reports
  `LOGIN: EXEC FAILED`. No successful 32K DSH boot is claimed.

The static KCORE+initial-VM lower bound now fits within 32K, but leaves
out dynamic kernel allocations, resident modules and other processes.
The next engineering step is to measure the actual physical allocator
failure and resident core occupancy after LOGIN starts. Further source
shrinkage should be based on real allocations, preserving functionality
and respecting the RAM-first optimization goal.
