# Idle-TTY kernel event hub, stage v3 (2026-10-13)

Stage v3 adds **disabled-by-default** kernel groundwork, not the live
idle-TTY dispatcher. Login and SUPERVISOR behavior remain unchanged.

The process layer owns three words: eligible idle-TTY mask, ready-TTY
mask and shared notification generation, plus the event latch. Physical
CTY/DCS/GE interrupt handlers now mark the appropriate logical TTY
without consuming its input mailbox. Child notifications also publish
to this hub. No per-terminal process or fixed input buffer is added.

TTYCTL suboperations 2..5 are restricted to **PID 1**, and respectively
install the eligibility mask, read the ready mask, read the generation,
and wait for the next notification. The waiter must first snapshot the
generation, inspect both ready terminals and child exits via WAIT/NOHANG,
then call IDLEWAIT; on return it must inspect both again. The kernel
rechecks generation after clearing the event latch and before sleeping.

The mask is zero at boot; no existing driver or supervisor must assume
an idle-TTY waiter exists. This release **does not** arm hardware scanners
without a reader. In particular DCS/GE receiver scanners still need a
dedicated idle registration/arming integration, including a policy for
multiple ready lines when their shared one-word mailbox fills. It is
unsafe to enable the idle mask and remove LOGIN processes until that
path is implemented and exercised under simultaneous input.

One further lifecycle prerequisite is terminal handoff: each first byte
must survive until AUTH reads it; successful authentication must EXEC a
shell in the same session, while failure and logout must reliably rearm
the correct TTY. This is not a tested behavior in v3.

Measurements from fresh PDP-6 builds before/after the dormant hub:

| Component | Baseline | First dormant hub |
| --- | ---: | ---: |
| KCORE image | 023076 | 023105 |
| KCORE BSS | 000524 | 000526 |
| CTY MRES image | 000067 | 000070 |
| DCS MRES image | 000074 | 000075 |
| GE MRES image | 000135 | 000136 |

These are **intermediate** measurements; the later PID-1 ABI and
per-TTY identification code further increase KCORE/MRES. Measure
against the complete rebuilt image before enabling the dispatcher.
The unchanged supervisor remains in one APR block.

Testing strategy: assembly/link for each kernel edit, then targeted
boot, LOGIN/LOGSTORE and interrupt tests. Full DSH conformance is a
milestone/end-of-feature regression rather than an edit-by-edit test.
