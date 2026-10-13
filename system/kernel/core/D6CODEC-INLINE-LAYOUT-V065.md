# D6 codec inline-token architectural experiment v065

Date: 2026-10-13. **Not production-integrated.** The v065 experimental
wire stream uses the old 36-token control-word layout but retains the new
18-bit descriptor (kind, exact length, distance). Each descriptor occupies
one full 36-bit token word, interleaved with literals. No group prescan,
paired-descriptor cursor, token-count header, or VFS suffix compaction is
needed: the existing D6LZ36 56-word streaming adapter works unchanged.

The native fixed-000060 v065 decoder accepts odd/even halfword matches,
word matches and zero runs, and uses the old streaming AC3/AC4/AC10..AC14
ABI. A native PDP-6 test passes distance-one overlapping halfword copying,
NEED_INPUT at a token boundary, and fixed-address installation. Another
native test links the existing D6LZ36 VFS frontend and passes repeated
2-word reads, including a 30-word literal stream. No BSS or decoder stack.

Measured native fixed decoder image: 74 decimal 36-bit words (octal 0112).
Together with unchanged old VFS adapter: 74+56=130 words. Baseline is
40+56=96, hence **34 resident words growth** before folding or whole-KCORE
linking, versus grouped v058+v064 198 words (+102). These are component
measurements only; neither variant meets the requested zero-net-growth gate.

Host-side re-packing of identical v045-policy tokens for the archived
81-executable DXR corpus (testkit `tests/host/d6codec-layout-v065/`):

| Metric | Paired v060 | Inline v065 |
| --- | ---: | ---: |
| Uncompressed DXR payload words | 159973 | 159973 |
| Compressed payload words | 131463 | 135534 |
| Growth over paired | - | 4071 (3.10%) |

The host model round-trips all 81 files. It does not represent an operating
system image benchmark or a full native encoder. The smaller decoder costs
DECtape/disk words and may alter execution throughput; neither has been
measured on live systems.

**Still required:** malformed stream and address-wrap validation, exact
completion and trailing-word rejection, all 18/36-bit match boundary cases,
end-to-end native encoder parity, Stage-1 installation on all devices,
whole-KCORE GCC/p10fold resident measurement and system boot testing.
Do not switch image formats on these preliminary results alone.
