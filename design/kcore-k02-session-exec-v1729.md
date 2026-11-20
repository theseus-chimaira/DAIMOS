# K02 combined physical session HUP, permanent D6FS and fatal EXEC v1729

Date: 2026-11-20. Status: full-system 256K PDP-6 guest PASS.
K02 and K04 remain OPEN. No production source or memory change.

## Ownership scenario and coverage

The v1728 native session test verified repeated HUP ownership, v1727
physically verified permanent D6FS refusal and private fatal-stack
backoff, and v1720 verified destructive low-memory EXEC metadata
ownership. v1729 runs all three behaviors simultaneously in one
booted PDP-6 with real RUN children, TTY, D6FS MRES, FILE, VM, MM,
fatal-close retry, session events and WAIT.

1. INIT (slot1) creates a controlling-TTY session leader (slot2).
   The leader starts job1 (slot3) and job2 (slot4) and reports PIDs
   through a genuine PIPE. Job1 is stopped via ordinary job control.
2. Job2 opens /SYSTEM/INIT on D6FS as FD3. Its first ordinary EXIT
   receives an injected refusal at the *actual* D6FS SYNC provider
   entry, retaining FD3. It then triggers a genuine invalid-user-APR
   fault and enters the real private-stack fatal FILE-close owner.
   Every subsequent targeted provider SYNC is refused. INIT waits
   until the production read-only GETTEARDOWN call reports this actual
   owner (stage 1), never forging FATAL_CLOSE or a FILE descriptor.
3. INIT delivers three HUPs to the session leader. The first uses the
   existing v907 test-time MM pin on job1's real u-area, so job1's
   HUP commits its irreversible VM_NONE+FINALIZE state without freeing
   the pinned extent. Job2's protected fatal FILE owner refuses the
   session teardown. Two later HUPs skip the already finalized job1
   and again refuse to touch job2's stack or FD3. The session's TTY
   cannot be retired on any of the three refused scans.
4. INIT independently RUNs an EXEC child (slot5). A copied staged
   loader returns NOMEM, forcing the real destructive low-memory
   replacement. Its old VM is physically destroyed and published as
   VM_SPACE_NONE. Only the replacement loader is then test-forced to
   fail. The actual fatal EXEC continuation exits and INIT WAITs for
   status 1. INIT's MEMINFO returns to its pre-EXEC baseline.
5. The final session HUP still refuses. Job2's actual stable u-area
   FD3 and FATAL_CLOSE bit remain intact; INIT emits the exact <P
   witness and executes its requested HALT.

At all three session-teardown failures, a test-only probe in the
copied proc_session_teardown_fail reads job2's live slot4 FD3 and
FATAL_CLOSE flag directly, and job1's slot3 TRANSITION/VM_NONE
descriptor. The D6FS hook compares the real canonical vnode of
job2 FD3 and checks its provider MRES lease against current slot4+1.
The four D6FS witness words are in copied *resident* fs_mres.s because
the provider itself is a movable MRES. Production VFS, FILE, EXEC,
session/event, scheduler, VM/MM and reaper code is unchanged.

## Physical 256K PDP-6 acceptance

| Witness | Observed |
| --- | ---: |
| Session leader / job1 / job2 / independent EXEC slots | 2 / 3 / 4 / 5 |
| Job1 u-area base (decimal) | 21,938 |
| Actual job1 MM pin / unpin | 1 / 0 |
| Initial member HUP / later member scans | 1 / 2 |
| Repeat HUP to finalized job1 | 0 |
| Premature TTY/session commit or owner errors | 0 |
| Session HUP refusals | 3 |
| Refused D6FS provider calls | 5 / 5 |
| Correct provider MRES lease / lease violations | 5 / 0 |
| All D6FS SYNC entry visits | 6 |
| Unsafe foreign job2 close / defer / close-fail | 0 / 0 / 0 |
| Low-memory EXEC NOMEM / post-VM-NONE | 1 / 1 |
| Replacement load refusal / fatal EXEC entry | 1 / 1 |
| EXEC VM/metadata ownership violations | 0 |
| Independent fatal-EXEC WAIT / MEMINFO baseline | PASS |
| Exact <P marker and HALT | PASS |

Test-only KCORE DXR SHA-256:
9291925accbec1b252ed2011b8a60749476ae0ee32aef9130c09fed878e45ddf

Physical SIMH transcript SHA-256:
ba11ea671d22929be2f0f5e1e0ac9a39b92898dc83d46231bc75d860a9d3adf5

The standalone full-system physical runner returned exit code zero.
Its exact verifier also passed in replay against the saved SIMH
transcript. Six independently modified transcripts were rejected:
missing completion, missing session refusals, unexpected session
owner error, missing post-VM EXEC witness, inconsistent provider
refusal count, and nonzero wrong-lease count.

## Testkit files and diagnostic history

Versioned testkit files:

- tests/validation/daimos-k02-session-exec-v1729.sh
- tests/validation/daimos-k02-session-exec-v1729.py
- tests/validation/daimos-k02-session-exec-v1729-hook.py
- tests/validation/daimos-k02-session-exec-verify-mutants-v1729.py

The guest is strict C89 and derived from the accepted v907 physical
session harness. Changes to assembler and failures exist only in
disposable kernel copies. The normal serialized filesystem gateway
is used for provider-originated errors. No production data, source,
field or ABI is changed.

Several earlier v1729 guest variants are intentionally not accepted.
The initial foreign-HUP test assumed that HUP immediately closed
job2 FD3. Physical counters showed that the safe foreign-event path
*deferred* the HUP, so the provider was never called. Fixing vnode
matching could not change this correct scheduler guard. A stopped-
member variant tried to WAIT for a grandchild's STOPPED report from
INIT, even though its parent was the session leader. Simply stopping
the member also failed to prove immediate foreign close. The
accepted scenario instead establishes the provider refusal through
job2's actual owner-side EXIT and APR fatal stack, waits for the real
GETTEARDOWN ownership state, and then tests the session HUP boundary.
No production defect is inferred from the earlier test mistakes.

Independent native regressions passed: v1728 permanent session HUP
with three unsafe mutants, v1720 destructive EXEC with two unsafe
mutants, v970 fatal owner / foreign HUP protection, and v922 repeated
session refusal with TTY retention.

## Memory and remaining K02 gates

Production KCORE executable words added: **0**. Fixed RAM added:
**0 words**. Per-process RAM/ABI added: **0 words**. No workaround.

Uncovered K02 conditions include an *organic* physical media/provider
failure rather than test-time rejection at provider entry; the
distinct physical backing-detach failure occurring **after** process
MM was already freed; and the final consolidated regression, resident
size and documentation audit. K04 must separately define recovery
policy for permanently unreleasable resources. An indefinitely
retained, safely owned resource does not prove reclamation. K03
automatic OOM termination remains disabled; unfinished work is not
propagated to pdp10-doc.
