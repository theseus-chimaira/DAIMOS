# K15 v1988: complete mode3 finalizer and physical PREVM retry

Parent 89d0255d. Validation only; original v1985/v1987 kernel,
MRES and KINIT bytes unchanged. No production code, resident TEXT,
DATA/BSS, stack layout or per-slot storage changes.

## Native integration

Testkit b93f4f6. New optional native fixture links full original
proc_finish_slot, idle finalizer, backoff, actual parent eligibility,
u-area MM release and executive-stack guard, real relocated mode3
DROP, VM destroy, common detach, MM pin/unpin/free. Outer session,
TTY, adoption and notification callbacks controlled (not actual
physical interrupts). Exact independent parent and MM owners.
GCC/KCC PDP6/KA10 each21 counted finalizer/reaper calls: one/two
pins, definite refusal, false reply, lost successful free, repeated
backoff, exact original bitmap bytes and span, one ZOMB publication,
no second HUP, source and u-area retirement, cleared retry word.
Actual overlapping reservation, saved executive PC, private wait
and live FILE refuse before terminal teardown. Eight executed unsafe
mutants per compiler reject: ignore VM refusal, no PREVM, lose span
escrow, no backoff, no stack guard, no user PC, no wait, no FILE.
Evidence relative TMPDIR: pure-bitmap-placement-v1985.4k1mr9 (KCC),
pure-bitmap-placement-v1985.NJA2fK (GCC).
Original v1681/v1982/v1985 regressions PASS
(pure-directory-prevm-regression-v1988.QZ1jYp), final default
v1985 runner PASS (pure-bitmap-placement-v1985.AtkBOG).
Initial fixture declaration/duplicate-symbol issues resolved,
short assembly alias to actual backoff avoids KCC long identifier
truncation. Existing GCC include-order warning only. No new
production toolchain defect observed.

## Actual 64K known-pin fault

Use accepted v1987 actual user TSTP/TERM/WAIT/EXEC workload. Same
12 children, slot4 first stopped bitmap terminal owner at octal
063000/251. Read actual original linked backoff MOVE/TLNN to
resolve private retry word. Introduce precisely one external
test-owned MM pin at the actual terminal entry. No other memory
deposits besides normal boot; no synthetic timer/return/flag,
payload or forced dispatch changes. First terminal refusal
publishes exact PREVM status0400001 and original span in BRK.
Actual idle slot0 backoff/retry reached at ticks8042/8074,
delay16->32->64. Both attempts retain entire MM extent set
(except introduced pin), original owner/pin, stable u-area, saved
user PC, backing/credential, exact span and terminal escrow,
all251 original bitmap words. No premature guest WAIT or FULL.
Remove only external test pin at second real idle terminal entry.
Original kernel retires owner and completes parent typed WAIT.
Other compressed TERM slots5/6 without restore; all10 stopped
TERM waits, slot3 restored actual marked-to-unmarked EXEC,
all12 exact EXIT waits, final MM/HALT PC0, unchanged root tapes.
Independent full verifier and11 corrupted trace controls PASS.
Physical trace does not sample retry word after successful scan
(native fixture does). Two preliminary replay cancellations before
fault injection: private retry symbol not exported (resolved from
actual instruction), then redundant current-PC breakpoint guard
detected (use different original finalizer/service entry guards).
No production bug established by these observer issues.
Evidence pure-directory-prevm-physical-v1988.IuWoyT,
physical.clean and audit-v1988.log. All replay processes stopped.

## Remaining recovery boundary

Known external pin PREVM fault gate accepted at actual 64K PDP6.
General K16 dirty FULL/DROP recovery, source loss, physical FULL
faults, RT latency/stack, real DSH/48K still open; default OFF.
Unlike RAW, exact directory encoded length is not derivable from
full VM size and omitted text. Preserve original encoded length
across a deferred transaction; never rediscover a possibly changed
descriptor length as authority. Design candidate (not implemented
or accepted): original BRK floor is initialized to aligned VM size,
current BRK may be unaligned and must remain exact. Original target
APR is also aligned. Their low alignment bits can hold exact length
and explicit phase without new persistent words. Bound encoded
length by original skip (text field plus origin, rounded down to
0200; maximum 040000 words), validate original BRK floor/current,
retain both original physical identities and prove clearing of all
transient bits on every healthy publication/cancellation. Do not
infer authority to consume unknown existing pins or accept source
loss. Compare with simpler zero-growth escrow designs before
implementation; no production change based on this candidate yet.
