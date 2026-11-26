# K15 v1983: source-built experimental directory PURE MRES

Parent 08b51176. The source-built package subgate is accepted, not
K15 or K16 lifecycle/physical acceptance. New explicit option
DAIMOS_PBM_DIRECTORY_V1983=1 requires strict swapless TSFS PURE:
DAIMOS_PURE_MRES_ENABLED=1, DAIMOS_SWAPLESS_ONLY_V1668=1, tape
boot, DAIMOS_PBM_V1667=0. Existing KA10 optional PURE boot guard
remains in force. Default OFF. No system executable marked by default.

Use the existing package and three exports (reclaim, slot-zero service,
terminal), select directory DROP v1979, FULL v1980, terminal v1982.
Link v1977 directory C and walker, v1979 atomic MM commit and v1664
finish in MRES only. Enable staged VM terminal gateway in this profile.
No extra trampolines, export slots, BSS or per-process fields.
Use existing mode-stamp scheme at both boot and kernel make levels,
including generated service/DROP/FULL/VM source and fold plan, so
changing representation regenerates code and package correctly.
Keep old v1667 linkage separate, reject mixed and nonstrict builds.

Testkit4e6dc7d, run make test-pure-directory-package-v1983. Actual
GCC and KCC directory -> RAW -> directory builds in same directory
produce byte-identical regenerated DXR, map and MRES package. Check
new symbols only in MRES, no old interleaved codec or terminal in
new package, incompatible settings refused. Evidence relative TMPDIR:
pure-directory-package-v1983.ATbVyg. Linked resident MRES GCC3258
->5359 (+2101), KCC3885->6401 (+2516). KCORE DXR image GCC024705
->024675, KCC025165->025155 octal (-8 each); BSS0415 octal unchanged.
Net +2093/+2508 resident words, zero new persistent per-process words.
Existing GCC include search warnings and old userland KCC notes remain;
no new observed compiler/assembler defect. Native RAW PI6 fault v1677,
reply v1808, EXEC handover v1934, stack lifetime v1937 and unsafe
controls PASS; logs pure-directory-native-regression-v1983.LzOloX.

Physical diagnostic: pure-directory-guest-v1983.jLrxmz, real source
build and TSFS validation (3 members,54 files,315 extents), independent
unmarked parent and marked compressed child linked by current host
DLINK, real d6block compressed payload. Source-derived capacity-driven
4..12 children, complete sparse array and exact WAIT status checks.
Read-only replay of private physical media, binds actual installed
service and terminal trampoline to MRES map; preliminary observer
requires mode3 ownership at pressure and restore/finish with pins,
final reaping witnesses. Initial observer refused relocatable entry
before starting simulator; fixed RH relocation. Bounded 150-second
replay pure-directory-physical-v1983.LOsJwC reached 64K, PURE LOADED,
DTFS LOADED, three child admission markers (unmarked then two marked), then
timeout before pressure. Shutdown triggered the pending B dump after
SIGTERM, not a genuine guest pressure marker. No verified mode3 owner, no complete runtime
trace or proof of failure. Final observer strengthened during run;
syntax checked, complete-trace oracle still unvalidated. No remaining
simulator process. Replay with a sufficiently long bounded diagnostic
limit and preserved private media; do not promote partial boot to K15.

Open: actual compressed scheduler/PI6 and finalizer lifecycle,
including EXEC, terminal while nonresident, exact WAIT/EXIT. K16 dirty
recovery, real RT latency (v1978 compression PI hold and live RT lease
RAW-fallback policy), stack/net-RAM/physical acceptance. Strict profile
is experimental, not default production mode3. No new runtime policy
selected by this patch beyond selecting staged chain for that profile.
