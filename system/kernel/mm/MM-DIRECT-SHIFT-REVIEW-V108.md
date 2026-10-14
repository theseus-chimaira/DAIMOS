# v108 — Direct descriptor relocation candidate (rejected)

Base is the latest v107 production MM, with 491 text words and 70 BSS.
`mm_move_extent` is 64 words. Investigated replacing the *post-success*
`mm_delete` plus insertion-index search plus `mm_extent_insert` with a
single in-place forward shift. The candidate traversed slots starting at
the original extent, copied following descriptors down one slot until the
next base exceeded the destination, and installed the saved extent with a
changed right-half base. Since this routine only moves **upward**, it did
not shift any descriptors before the original slot.

Assembled candidate: **498 text words, 70 BSS** (whole MM), **+7 text**
versus baseline. `mm_move_extent`: **71 versus 64** words. Production
MM and extracted production VM/memory-copy assembler ran with the v106,
v105, v099, v094 and v095 native PDP-6 suites: **all PASS**. This is a
correctness-positive but *size-negative* experiment, not a performance
benchmark; CPU instructions, PI latency, and stack have not been measured.
The new direct shift also bypassed the normal insertion helper's capacity
checks, making the proof obligation more difficult despite equal descriptor
count. The candidate was **reverted**, leaving the production allocator
unchanged at 491 text + 70 BSS.

An in-place transfer might still be faster in isolation because it moves
fewer words, but we should not trade seven resident code words for an
unmeasured speed benefit on the PDP-6. Further tests should compare
execution counts and consider small helper-level improvements without
altering MM's successful VM publication and PI-critical transaction.

ITS-138/JOSS remain historical references, not drop-in word-granular
allocator replacements. No historical superiority is claimed.
