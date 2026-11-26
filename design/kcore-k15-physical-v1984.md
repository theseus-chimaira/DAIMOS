# K15/K16 v1984: actual directory pressure and restoration

Parent a3de5036. Actual v1983 source-built GCC strict swapless TSFS
64K PDP-6 SIMH, no AUX backstore, private tapes, no guest deposits.
The matched RAW build uses byte-identical unmarked INIT and independently
linked marked compressed COMP; both three-member roots validated.
The current RT fallback patch follows those physical builds, so do not
attribute the old physical measurements to the corrected source.

## Physical evidence

Independent testkit verifier checks image hashes, genuine checkpoint,
exact u-area/slot/map, all MM arena bounds/ordered nonoverlap, source
DXR text boundary and actual relocated service/terminal entry points.
At pressure B both graphs contain only sleeping nonresident children:

| Metric (decimal words) | RAW | Directory |
| --- | ---: | ---: |
| Admitted children | 9 | 8 |
| Full APR | 4096 | 4096 |
| PURE prefix | 512 | 512 |
| RAW owners | 9 x 3584 | 1 x 3584 |
| Bitmap owners | 0 | 7 x 239 |
| Total free | 5908 | 31078 |
| Largest 1024-aligned free span | 1536 | 3584 |
| Additional resident package plus fixed KCORE | 0 | 2093 |

Equivalent same-child geometry: 7*(3584-239)=23415 gross; 21322 net.
This is actual physical length evidence but not a claim that the
free fragments are usable APR space. Measured capacity REGRESSION -1.
The original mode3 bitmap source remains at its old aligned APR base;
actual directory bitmap bases (octal) 070000,100000,...150000.
Between adjacent bitmaps only3857 words physically free, 3072 words
after APR alignment. Maximum free aligned3584, less than required4096.
The RAW child first restores/exits; then actual directory FULL entry
PC041661 with slot3, slot0 current, TRANSITION held, original unpinned
source150000/239, exact u-area. Actual atomic finish PC045433 has
same original source and disjoint APR160000/4096 each pinned exactly
once. Peak image storage4335 words. After finish all eight directory
children emit w and parent Z after exact WAIT/EXIT status and sparse
array checks; all nine RAW children likewise emit w and parent Z.
Both original observers mistakenly reject concatenated ZV1983_Z_STOP
and stop before the last MM/HALT. Corrected parser handles directory
and RAW prefixes; full corrected-observer replay remains outstanding.
Independent actual transcript verifier and eight falsified traces
PASS (fake pressure, shutdown dump, missing wait, wrong mode/target/
slot/current and missing source pin). This proves guest-level completion
and a real mode3 restore/finish, not final MM, physical PI6/READY_ONCE,
nonresident terminal or EXEC acceptance. Neither simulator left running.

Evidence relative TMPDIR:
- pure-directory-guest-v1983.jLrxmz/build
- pure-directory-physical-v1984.fQfzgb (verified-v1984.json.txt,
  verifier-controls-v1984.log, physical.clean)
- pure-directory-raw-control-v1984.J5WhuQ/build
- pure-directory-raw-physical-v1984.Oaz4D2/physical.clean

## RT policy and compiler validation

Previous shared pbmp_owner_v1978 rejected all live RT leases, including
clean RAW fallback. Move RT exclusion to compression prepare and exact
lease validation. Original stable owner proof still precedes fallback;
existing RAW transaction unchanged. No long codec passes while RT
lease active, no new policy governing which ordinary RAW owners are
eligible. Test RT lease on victim or another slot, no codec/bitmap
commit, lease unchanged, actual RAW rebase; both compiler native K13
49 cases per CPU, eight unsafe mutants (including allow RT codec and
incorrect RAW refusal) reject. K12 native KCC72/CPU and six mutants
PASS. KCC DROP1281->1283, GCC1035->1037, default KCC691 byte-identical.
No fixed KCORE/per-slot/DATA/BSS growth. The two-word correction is
not included in original v1983 measured net cost.
GCC compile selection for existing native transaction runners:
FULL52, service46, terminal65 plus staged VM65, each CPU; all17 mutants
reject. Existing GCC include-order notes, no new observed toolchain bug.
Evidence pure-rt-fallback-v1984.sOhu8X, pure-bitmap-full-v1980.PU4Vid,
pure-bitmap-service-v1981.1M48YW, pure-bitmap-terminal-v1982.yfQrGy,
pure-bitmap-commit-v1979.A6ckyl and E0NsnJ, pure-bitmap-prefix-v1978.pXFaGf.
Testkit13aae53.

## Next placement gate (not implemented)

Fixed mm_compact scans only PROCESS; RAW and directory share type4
PACKED_PROCESS. Do not make type4 generically movable. Investigate a
separate exact mode3-only consolidation operation on inactive unpinned
owners, existing TRANSITION and MM proof, no speculative free or
foreign writes. Clean mode3 has no resident APR or recorded physical
source outside its MM descriptor. Existing word067 source sector
escrow supports 0200 alignment, so 0200 rather than 02000 source
alignment may allow dense packing without per-slot storage. Restore,
terminal, retry and deferred recovery must all agree on geometry and
original identity; RAW still needs its original prefix geometry.
Do not choose relocation or in-place expansion until compare their
RAM/code, copy cost, failure atomicity and measured aligned capacity.
Never move pinned/dirty/terminal escrow owners. Physical acceptance
requires improved admitted APR capacity and exact runtime ownership,
not just large aggregate free count. Continue K15 complete PI6,
READY_ONCE, EXEC, nonresident terminal/final MM/HALT and K16 uncertain
recovery, 48K/64K/real DSH, latency and stack measurements. Default OFF.
