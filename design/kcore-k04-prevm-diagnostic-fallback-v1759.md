# K04: exact PREVM diagnostic precedence v1759

Status: source diagnostic correction; isolated PDP-6 DAS PASS.
K04 remains OPEN for physical fault-injection acceptance.

The v1755 opt-in GETTEARDOWN precedence fix bypassed the historical
fatal FILE-stage check whenever a descriptor had TRANSITION, a live
VM and a stable u-area. It then tested FINALIZE|PREVM and returned
stage NONE if either mark was absent. Consequently an inconsistent
or unmarked descriptor retaining PROC_FATAL_CLOSE_BIT could be
misreported as NONE rather than the existing FILE stage.

v1759 requires both FINALIZE and PREVM marks *before* bypassing the
FILE-stage check. If either mark is absent, the original FILE-first
diagnostic executes. An exact EXIT PREVM owner still takes precedence
over the stale fatal-close bit. Legacy queries are unchanged.

No process state, ABI, retry timing or persistent storage changes.
The kernel assembled with DAS; no physical runtime test is claimed.
