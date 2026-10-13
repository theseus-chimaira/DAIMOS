# D6 codec VFS compaction v064 — native size experiment

Date: 2026-10-13. The incomplete-group suffix in the existing 64-word
stack window is compacted with one PDP-6 BLT rather than an explicit
per-word loop. BLT advances the source and destination pointers forward;
source is always at or above the destination, so overlapping regions are
safe. No additional stack or static storage is allocated.

Standalone native linked sizes (entry to appended end marker):

| VFS implementation | Octal span | Decimal words |
| --- | --- | ---: |
| Original D6LZ36 | 001104..001174 | 56 |
| Grouped v058 | 001235..001344 | 71 |
| Grouped v064 | 001235..001342 | 69 |

The v064 adapter saves two code words against v058. Its combined size with
the fixed v058 decoder is 198 words versus the legacy 96 (+102). Combining
v064 with the guarded v062 decoder would occupy 208 (+112) words before
post-p10fold KCORE linking, so neither combination meets the no-growth gate.

The testkit v064 native PDP-6 VFS regression passed for repeated two-word
reads including a maximum-length 31-word group. This is an experimental
optimization, not a production format cutover. Original v058 files remain
unchanged so measured variants can be compared directly.
