# K15 v1987: actual nonresident terminal and restored EXEC

Validation only, parent 865ea7e8. No production kernel changes.
Testkit1b0942c reuses the actual v1985 GCC source-built strict
swapless TSFS 64K PDP-6 image. KCORE, PURE MRES and KINIT unchanged;
only separately linked user fixtures and TSFS root tapes replaced.
No new kernel hooks, artificial interrupt or runtime deposit.

## Guest and observer

Unmarked INIT creates unmarked PIPE child2, marked compressed
PURE DXR2 PIPE child3, and marked user-mode spinners. Spinner
signals readiness through a distinct PIPE, closes that descriptor,
then executes only user instructions. Parent sleeps before real
authorized TSTP, checks STOP and exact typed STOPPED wait.
At actual pressure, saved user PC and absent executive wait class
required for every stopped child. Terminate via actual authorized
TERM, not by resuming it or manipulating MM. Parent checks exact
EXITED/event TERM status. PIPE children then wake; child3 checks
all sparse data, calls actual EXEC of separately linked unmarked
PURE POST. POST checks PID/argc and returns distinct exit056; plain
child2 returns057. Parent checks both exact typed waits and data.

The actual physical observer guards early mode3 restore before TERM
waits. At relocated terminal entry, require original exact MM owner,
mode3, stable u-area, TRANSITION, original saved user PC/identity,
no active executive reservation, no live FILE or user map. After
each typed WAIT require absence of exact child user/u-area owners
and retired storage mode. At L all stopped children retired,
slot3 still has its original bitmap. Actual FULL and atomic finish
require exact pinned source and destination; EXEC entry requires
committed resident mode0, old marked record, cleared escrow/backing.
At real proc_exec_enter require same slot/u-area, changed backing
record, cleared mutable eligibility, exact sole new resident APR,
no outstanding EXEC reservation. Actual guest X, Z and final MM
and normal HALT. Offline verifier independently checks trace order,
actual link bindings, DXR2 headers and unchanged root tape bytes.

## Accepted result

Actual pressure12 children: slot2 RAW3584, slots3..6 bitmap251 each,
slots7..13 resident4096. Slot4/5/6 foreign TERM from original
octal063000/064200/065400, each251 words (753 total). No restore.
All10 STOPPED and subsequent TERM/WAIT checks; no surviving child
user/u-area owner. Slot3 source061600/251 restored to160000/4096,
actual EXEC000147 then proc_exec_enter024311; new unmarked
2048-word APR, same slot/u-area, new record, X. All12 exact exit
waits, final MM and mode map, normal HALT, three root tapes unchanged.
Independent verifier and eleven corrupted trace controls PASS.
Evidence relative TMPDIR:
pure-directory-lifecycle-build-v1987.Qwey5T/build,
pure-directory-lifecycle-physical-v1987.JkKVum/physical.clean,
verified-final-v1987.txt, trace-controls-v1987.log.
TSFS members3 files55 extents316. Makefile audit target re-PASS.

Native v1934 EXEC, v1982 terminal (PDP6/KA10 staged/default),
v1985 placement (PDP6/KA10) regressions PASS. Historical v1681
test extractor lacked actual v1937 stack guard; link original
guard and add overlapping reservation refusal, without production
source changes. Five executed unsafe mutants reject including
bypassed stack guard at040. Evidence
pure-directory-lifecycle-regression-v1987.1rHs4R and
daimos-pure-terminal-finalizer-v1681.3zLbCj.

Healthy 64K actual EXEC and foreign nonresident terminal accepted;
combined with v1985/v1986, healthy K15 lifecycle witnessed.
K15 remains open for physical PREVM failure/retry and repeated
failure-boundary integration. K16 dirty recovery, source loss,
RT/PI latency and stack measurements, real DSH/48K acceptance
remain open. Native fault controls do not establish physical
fault acceptance. Default OFF. No production size changes.
