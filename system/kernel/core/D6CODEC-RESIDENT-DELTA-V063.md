# D6 codec fixed + VFS linked-size measurement v063

Date: 2026-10-13. PDP-6 native `p10run` deposit execution succeeded for both
self-contained size-link harnesses; map symbols are the authoritative
addresses. Each harness links a stub VFS reader and the same stack CRT,
followed by a complete fixed decoder and its VFS runtime frontend. End
markers were appended to temporary copies of the VFS frontend without
changing the original source; no extra executable words were inserted.

| Component | Old D6LZ36 | Grouped D6 codec | Delta |
| --- | ---: | ---: | ---: |
| Fixed decoder (v058) | 40 | 129 | +89 |
| VFS adapter, linked (v058) | 56 | 71 | +15 |
| Combined, decimal 36-bit words | 96 | 200 | **+104** |
| Combined with address-guarded v062 decoder | 96 | 210 | **+114** |

Old VFS map: entry 001104, end 001174, difference 000070 octal (56 decimal).
New v058 VFS map: entry 001235, end 001344, difference 000107 octal
(71 decimal). The v062 fixed decoder is 139 decimal words; its VFS adapter
is not yet separately optimized or production-linked.

This is a controlled component-level text-size comparison, NOT a measurement
of a final KCORE image or a post-p10fold difference. No codec candidate
currently meets the stricter zero-extra-resident-code goal. Decoder data/BSS
and persistent refill RAM are unchanged. Do not switch Stage-1 producers or
KCORE runtime callers until a genuinely budget-compliant complete decoder
and VFS arrangement is validated by native regression and a full lowmem link.
