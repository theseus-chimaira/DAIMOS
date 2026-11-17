# PDP-6 KCORE v1658: permanent MM uncertainty and PURE owner disposition

Date: 2026-11-17. Source base: v1657 DAIMOS 2370e0b1.
Scope: opt-in PURE RAM MRES, exact PDP-6 MM/process lifecycle.
Strict 32K acceptance, size-only optimization, and compressed mode3
are deferred. PURE remains OFF in the default kernel.

## Existing ownership model

The one authoritative MM extent table describes each PROCESS or
PACKED_PROCESS physical owner with base, size, type, slot and pin count.
The existing two-bit storage mode and the process TRANSITION bit control
eligibility. There is no second registry, new process state, per-owner
buffer or persistent RAM word. The saved backing identifies immutable
executable text; the retained suffix contains authoritative mutable data.

The scheduler excludes TRANSITION before dispatch, real-time selection,
swap exchange and victim reclamation. The PURE MRES never issues READY_ONCE
unless restore returned success, TRANSITION cleared, a real APR exists and
RESIDENT was published. TERM/HUP cannot bypass an in-progress TRANSITION.
The idle finalizer accepts only the separate FINALIZE-marked PREVM or
post-VM teardown states; it must not interpret a PURE transition as an
EXIT continuation. A permanently uncertain PURE owner consequently remains
an unavailable process slot with its allocated extents reserved. This is
an explicit fail-closed quarantine, NOT an automatic recovery mechanism.

## Transaction and disposition matrix

| Operation / refusal | Authoritative state | Safe disposition |
|---|---|---|
| DROP source pin refusal | Full PROCESS/APR, unchanged | Retry only after one exact unpinned resident owner proof |
| DROP rebase validation/refusal | Full PROCESS, pinned until cleanup | Unpin, validate exact owner, release TRANSITION only if unpinned; otherwise quarantine |
| DROP final suffix unpin refusal | PACKED suffix and saved executable | Exact singly owned and UNPINNED PACKED proof may unlock; otherwise quarantine |
| PREFIX source pin or prefix claim refusal | PACKED suffix, no mapping | Release validated pin; exact unpinned PACKED may retry or use FULL |
| PREFIX loader failure | Claimed full-span PACKED and pinned | Release prefix, then unpin suffix, with both checked; retained RAW if successful |
| PREFIX prefix-release/unpin refusal | Full-span or suffix PACKED remains owned | Quarantine when exact unpinned suffix cannot be proved |
| PREFIX final MM commit | Full-span PACKED pinned exactly once | v1658 validates, retypes and consumes pin in one infallible descriptor store before APR/RESIDENT/TRANSITION publication |
| PREFIX commit validation failure | Full-span PACKED and pin | Try checked release and unpin; otherwise quarantine |
| FULL source pin/allocation/target pin/loader error | Original PACKED RAW retained | Checked destination release then source unpin; exact unpinned RAW may retry |
| FULL final target unpin refusal | Original PACKED RAW still authoritative | Checked rollback of target; permanent refusal retains one or both pinned descriptors, no publication |
| FULL source unpin refusal | Original PACKED RAW still authoritative | Free only independently unpinned provisional target; retain uncertain RAW pin under TRANSITION |
| FULL source free refusal | Original RAW remains unless MM accepted free | Free provisional target; allow retry only if exact one unpinned PACKED extent survives; otherwise quarantine |
| FULL target cleanup free refusal | Original RAW and provisional target still MM-owned | Never reuse extra PROCESS descriptor; TRANSITION held, no runnable APR |
| Terminal PACKED detach free refusal | Exact PACKED source and process u-area | Keep slot, backing, storage and FINALIZE/PREVM owner; retry via existing idle backoff only for proven teardown markers |
| Post-free mode publication mismatch | MM source already deleted, storage inconsistent | Existing kernel storage invariant fail-stop; never silently reconstruct an owner |

None of the checked MM primitives has blocking I/O in the short PI-disabled
commit region. The loader and independent DTC3 reads execute with PI enabled
and both extents pinned. Never attempt forced unpin, infer a missing owner,
decrement a pin without proof, or change mode with a bad MM descriptor.

## v1658 changes and costs

The original fast prefix commit used to retype the descriptor to PROCESS,
publish RESIDENT and only then call the fallible mm_unpin(base). v1658's
`mm_pure_commit_prefix_v1384` requires exactly one pin and changes both
type and pin count in ONE metadata update. `proc_pure_restore_v1384`
publishes its APR and clears TRANSITION under the same PI-disabled region.
The fallible postpublication MM call disappears. The existing prefix
rollback is unchanged and remains checked.

For DROP, the real MRES service now checks an exact unpinned PACKED owner
after any rejected DROP (in addition to an unchanged unpinned RESIDENT).
This handles the narrow case where the suffix unpin completed but its
return was lost, without unlocking a genuinely pinned or duplicate owner.
The error-only extra owner proof has no ordinary-path cost and needs no
persistent storage. On successful exact PACKED proof, compaction is retried
once, not in a spin loop.

The new source-linked native PDP-6 testkit v1658 executes original v1654
FULL and v1612 service suites, plus fast-commit no-pin, completed-unpin-
but-reported-error and genuinely refused-pin cases. A native, assembled
and executed mutant omitting the new MM pin-consumption instruction must
fail the live owner oracle. These checks PASS on the current v1658 source.

Permanent uncertain pin/owner faults are not hardware recoverable from the
known descriptor contract. They intentionally quarantine rather than
inventing a synthetic EXIT, reusing a descriptor or halting unrelated
processes. A restart is required for an irrecoverably quarantined process;
repeated refused operations must not spin the PI6/slot-0 scheduler.

## Acceptance limits and next gates

Fresh 48K source-built GCC TSFS-root/secondary-DTC3 multi-owner guest
completed original PIPE/WAIT/EXIT with normal HALT PC 000000; its
fixed KCORE.stream SHA256 is
88970ee633dc22094565172348bc1dcb9f49ba0417e104fb5608b71a87668d01,
byte-identical to the previously accepted v1654 core. KCORE low end
024253 octal, opt-in PURE DXR image 003355 octal words (BSS 0), KINIT
image end 113641 octal (eight decimal words above v1654's 113631).
This is a matched fixed-core SHA, not a 48K ON/OFF RAM comparison.

Independent original physical 48K FULL DTC3 read failure now returns -4
at the source-linked v1658 loader, retains PACKED RAW, releases the
provisional FULL target, restores the SAME reel and completes six
WAIT/HALT. Original source auditor rejects both forged source-pin and
fake successful-I/O traces; the end-to-end v1658 runner exits zero.
This is physical I/O refusal, not physical MM descriptor corruption.

New 64K regression, permanent terminal/cancellation refusal injection,
continuous whole-machine RAM high-water, matched PURE
OFF/RAW comparison, compression state3 transaction and full frozen
release matrix remain open.
The v1657 19,770-word bitmap gain is only a data-only candidate.

No permanent KCORE object, kernel storage map, process struct, MM descriptor
format or syscall ABI was modified in v1658. Exact optional MRES text
change must be recorded from the linked source-built map, not projected.
