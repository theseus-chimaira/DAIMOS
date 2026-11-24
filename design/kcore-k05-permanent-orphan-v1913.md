# K05 v1913: last orphan remains quarantined without foreign-owner repair

Date: 2026-11-24. **Original source-built 48K PDP-6 physical PASS, scoped.**

## Terminal policy under unresolved MM ownership

v1912 proves normal last-child EXIT/HALT only AFTER a test-induced
foreign MM owner can be authoritatively reconciled. The final
irrecoverable state has fundamentally different ownership rights:
slot3 retains its PACKED RAW suffix, an unretired temporary full
PROCESS target belongs to a foreign owner2, and the private
executive continuation is still live. Its saved stack cannot be
destroyed safely merely because there are no ordinary runnable
users. The existing conservative KCORE policy is to **remain in
slot0 idle, retry with bounded backoff, and preserve the owner**.
There is no OOM-killer or automatic guessing of MM owner identity.

The original `proc_finish_service_v881` scans failed owners and
preserves the shared pending bit; only a completed physical
cleanup can qualify as last-user release. Original
`proc_has_live_user` scans compact process state and sees the
live TRANSITION slot. `proc_idle_loop` invokes
`proc_exit_halt` only upon real last-user completion, not
absence of a runnable APR. That distinction is essential:
last-runnable and last-live are not interchangeable.

## Extended original PDP-6 physical witness

The unchanged v1910 five-child guest and original fixed KCORE/
optional PURE pass the complete v1911 prefix physical oracle:
real PACKED source `mm_free=-5`, target owner3 -> foreign2,
refused parent TERM, actual parent EXIT and healthy child
slots4/5/6 EXIT, MM10 sole live child3, three capped native
retry scans and two original true PCLK PI6 observations.

v1913 adds **twelve more native**
`proc_finish_service_v881=013065` entries, each followed by
a distinct original slot0 `proc_idle_loop=017552` return.
Physical clock at extra retry1 is `027342` octal and at extra
retry12 is `042342`, with **exactly `01000` (512 decimal)
clock ticks** between each neighboring native retry and from
the preceding v1911 retry3. Shared pending remains armed and
the retry delay remains at its `01000` tick cap throughout.
Two more actual original PCLK PI6 at fixed KCORE `017225`
save original hardware words `340000017553` and
`340000017564`, both interrupting the real slot0 idle loop.

The **complete ten-entry physical MM owner table** is
byte-identical at every new retry entry/completion and PI6.
Authoritative RAW source `111200..120000` owner3 pin0 and
foreign temporary PROCESS target `120000..130000` owner2
pin0 (after MM compaction metadata `024707`) remain allocated.
The original ROOT, slot3 `047017600001` orphan TRANSITION/
VMbase0, private executive CTX_PC `340000016704`, FDCTL
`000000004010`, exact uarea word067 escrow `040000120000`,
and all other FREE user slots remain unchanged. No foreign
target repair, speculative target free, premature transition
clear, forced FINALIZE or native guest HALT occurs. SIMH ends
only by explicit operator stop after the final real PI6.

In combination with v1911, **15 actual capped idle retries**
are witnessed under the same permanently unrepaired owner.
Four independent unsafe physical trace mutations reject
false owner repair, premature transition-clear, lost escrow
and incorrect original 512-tick PCLK cadence.

Original fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.
Accepted normalized physical trace SHA256:
`88867609a4329cd0a352f9681a3c59e34afe24b557ed20c3354d84918a0b7529`.

Testkit: `daimos-testkit/tests/validation/pure-k05-permanent-orphan-v1913/`.
Permanent documentation: `pdp10-doc/KERNEL/PURE-K05-PERMANENT-ORPHAN-V1913.MD`.
Accepted evidence: `$TMPDIR/daimos-k05-permanent-orphan-v1913-physical-v1/`.

**Acceptance scope:** twelve additional real retries plus two
PI6s are a finite observation, NOT a mathematical guarantee
of arbitrary infinite uptime, all timer wraps or recovery
when the MM ownership cannot be reconciled. This establishes
the current fail-closed terminal policy in the observed
ordering. Permanent owner cleanup and further allocator/PI6
and platform cases remain K05 OPEN; K06-K09 OPEN. RAW PURE
and mode3 remain default OFF. Production source/IMAGE/BSS/RAM
unchanged.
