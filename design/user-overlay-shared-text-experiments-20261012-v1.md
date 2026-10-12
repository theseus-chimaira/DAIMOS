# User overlays and shared-text experiments (2026-10-12, v1)

Status: implementation plan only. No runtime implementation or acceptance claim.
Priority: overlays first; shared execution areas and data-state optimization later.
Baseline: current independent PDP-6 user extents and DXR loader.

## Goal and decision gates

Measure resident words, peak core, overlay I/O, latency, and CPU overhead
against an unchanged DAIMOS baseline. Preserve current ordinary EXEC and
process behavior as the default and as a fallback. First measure DSH and
representative native tools' resident working sets. Only add overlays when
measured savings justify the additional loader, ABI and runtime complexity.
Tests belong in daimos-testkit, not the DAIMOS source tree.

## Phase 1: standalone user-space overlays (first implementation)

1. Profile DSH text, initialized data, BSS, heap, stack, peak resident
   memory, routine frequency, and blocking/preemption sites. Include native
   MAKE, DAS, DLINK, editor, and KCC candidates before generalizing.
2. Establish a versioned executable overlay manifest: fixed logical window
   offsets, overlay IDs, word lengths, persistent entry stubs, entry points,
   relocation records, backing identity and integrity/version checks.
   Preserve DXR1/DXR2 compatibility; reject malformed layouts.
3. Keep the overlay dispatcher, loader entry points, and return trampolines
   outside any replaceable window. Record nested calls and outstanding
   returns. An overlay cannot evict code that an active continuation still
   needs, unless the continuation will safely reload it before resumption.
4. Handle interrupt/preemption at any instruction, including inside overlays.
   Every process has its own logical overlay state in phase 1. No implicit
   assumption that reaching a command boundary makes a buffer disposable.
5. The loader verifies overlay relocation, code/data separation, entry
   range, storage failure handling, and transactional publication. Do not
   overwrite the last usable code image before a replacement is validated.
6. Leave physical placement, overlay activation and eventual mapping policy
   to replaceable VM backends. The generic executable layer may name overlay
   objects and logical windows but must not hardcode PDP-6 APR assumptions.
7. Account for a contiguous PDP-6 extent, 02000-word alignment, BRK growth,
   VM compaction, whole-image/process PURE swapping, sleeping kernel returns,
   and held user mappings. No movement or eviction with live physical aliases.
8. Measure overlay misses, bytes/words loaded, storage wait time, maximum
   active overlay depth, memory saved, and end-to-end workload runtime.
   Include persistent backing availability when files are replaced/unmounted.

### Rules specifically to preserve future shared-text support

- Identify an overlay by immutable executable image identity/version plus
  overlay ID, not by process ID or a raw physical address.
- Keep logical entry addresses fixed and let each VM backend choose placement;
  avoid embedding physical bases in the user ABI or persistent overlay format.
- Explicitly separate immutable shareable code, mutable per-process data,
  mutable per-user data (only when semantics allow), and per-activation scratch.
- Relocations, text writes, runtime patching, writable literals, and pointers
  from shared text into private data must be audited; mark non-pure overlays
  ineligible for sharing rather than silently sharing them.
- Preserve per-process overlay call/return state. Shared execution areas may
  have only one currently loaded overlay; an owner's suspended PC may require
  restoration before that process runs again. Do not evict a live return path.
- Allow overlays that cannot safely share to stay process-private; support
  normal non-overlay executables unchanged.

## Phase 2: PDP-6 common execution-area experiment

Experimental, opt-in, explicitly unprotected across shared participants.
One immutable program text copy plus one mutable execution window. Multiple
processes keep independent saved user data/heap/stack and stable u-areas.
On ownership change save the outgoing private window and restore the incoming
window using BLT; on scheduler switches to other programs do nothing unless
the common window's owner actually changes. Retain the process's original
logical addresses for all restored C pointers. Do not activate a process
until both its data and the overlay required by its suspended PC are present.

Make the shared-area object an owner of its physical allocation; make process
attachment, EXEC, exit, swap-out/in, BRK resize, compaction and backing
retention explicit and transactional. Never replace live user bytes while a
kernel continuation or held mapping can still reference them. Reject
programs whose writable state cannot be safely exchanged. Start with a
single area; multiple areas/cache policy are a distinct follow-up experiment.
User-to-user isolation is intentionally not promised in this experimental
mode. Keep it disabled by default.

## Phase 3: optimized private-state and overlay combination

Inspect DSH global objects, parser and RUN workspaces, history, variables,
aliases, jobs, libc state, and stacks. Use shared immutable objects wherever
correct; share per-user objects only with explicit synchronization and
defined visibility rules. Keep environment, jobs, open operations and live
parser state private unless proved otherwise. Scratch storage can be omitted
from save/restore only at formally safe continuation points; timer preemption
can occur anywhere. Maintain an unoptimized full-save variant for comparison.

With overlays enabled, track both execution-area owner and overlay resident
identity. Charge BLT copying separately from overlay reload I/O. A saved
process whose PC is inside a replaced overlay must reload that version before
resuming. Nested overlay calls need persistent return trampolines/frames.

## VM boundary

Keep generic process/scheduler policy unchanged; expose a small capability-
checked VM prepare/activate contract and abstract overlay/image handles.
PDP-6 uses a contiguous relocated execution window and BLT state exchange;
KA10 may use its supported segment mechanisms; paged systems can map common
text pages. No assumption that those backends implement PDP-6 copying.

## Performance/acceptance matrix

- Baseline, overlays-only, shared-full, and shared-optimized variants.
- 1, 2, 4, 8 interactive shells; same and distinct executable images.
- Default 60 Hz clock / 4-tick quantum, plus adversarial every-tick handoff.
- Blocking TTY, pipes/background jobs, nested overlays, partial I/O, EXEC,
  exit, memory pressure, compaction, BRK, swap and storage errors.
- Record: peak/steady resident words; immutable text duplication; words
  saved/restored; actual ownership transitions; BLT elapsed machine cycles
  where supported; overlay hits/misses, I/O latency; scheduler latency;
  throughput. Keep hardware-time estimates separate from simulator wall time.
- Require repeatable target-simulator and testkit regression passes before
  enabling any experiment by default. Document unsupported cases explicitly.

## Stop conditions

Do not proceed to shared text if overlays alone already create unacceptable
I/O latency or complexity. Do not make scratch buffers globally shared when
their contents can be live at preemption. Do not expand generic kernel
resident code merely to accommodate a PDP-6-only optimization without
measured net core savings.
