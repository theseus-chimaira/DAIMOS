# INIT resident-memory optimization on 32K PDP-6

**V2 continuation:** the resident supervisor is now a separate executable
that replaces startup INIT in **PID 1** via transactional EXEC. Its final
text is 001530 octal words and BSS 000461, resident extent 006000.
See `design/INIT-SUPERVISOR-PID1-SPLIT-20261012-v2.MD` for the complete
argument-handoff protocol and overlay rationale. This older v1 document
records the first libc splitting experiment and its measurements.

Date: 2026-10-12. Basis: `userland/init/init.c`, the actual PDP-6
KCC/DAS/DLINK DXR map, and the low-memory EXEC allocator implementation.

## Goal and invariants

Retain PID 1's complete behavior: configure startup storage policy, run
MOUNTALL, parse eight INITTAB entries, launch LOGIN and other configured
services, reap children, log failures, and respawn RESPawn-designated
entries. The eight-entry capacity, TTY mapping, policy semantics, and
child/exit handling must remain unchanged. Prefer permanent RAM savings
over code speed and disk size.

## First measured reduction: split the unneeded S6REC reader

`INIT` reads configuration with `u_text_getline()`. It never calls
`u_text_gets6()`, yet both were linked from the same `libc/text.c` object.
The latter is now in `libc/text_s6.c`, and INIT's source list excludes
that object; DSH and other users retain the complete S6REC interface.
The word-reader cursor is shared rather than duplicated.

Fresh KCC/DAS results (octal 36-bit words):

| | Before | After |
| --- | ---: | ---: |
| INIT DXR image | 005374 | **005222** |
| INIT BSS | 000536 | **000536** |
| Executable + BSS + 002020 origin/stack | 010152 | **010000** |
| 02000-word-rounded VM extent | 012000 | **010000** |

The saved **0152 octal / 106 decimal image words** cross one physical
VM allocation boundary, saving **02000 octal / 1024 decimal resident
words per INIT** without changing INIT source behavior. Other userland
programs continue to link `libc/text_s6.c` as required.

## INIT's lifetime and further optimization

Startup-only operations: parse `/CONFIG/SYSTEM`, activate storage policy,
spawn and wait for MOUNTALL, read `/CONFIG/INITTAB`, and launch services.
Permanent operations: wait/reap child processes, identify INITTAB slots,
log failures, and respawn services. `spawn_entry()` and all eight
configured entries must remain available after startup.

INIT still keeps the startup parser, policy reader, and mount-launch
code in its resident DXR. Next measurements should compare (a) further
libc linkage splitting, (b) moving startup-only services to a separate
setup executable with a correctly specified configuration handoff, and
(c) an overlay-aware text layout. Never remove code that a child-respawn
or startup-error path can reach.

## Overlay feasibility, not yet implemented

PDP-6 physical core uses contiguous process extents and does not map
shared executable text. User text overlays would instead reserve a
single resident window and load mutually exclusive code groups there.
Possible startup-only INIT overlay: SYSTEM/INITTAB configuration parsing,
storage policy setup, MOUNTALL orchestration. A resident supervisor must
keep `wait`, child reaping, respawn, error logging and process launch.

Requirements before an overlay format or loader is accepted:

1. Extract actual linked function sizes, call graph and relocation needs;
   compute net savings after adding overlay window, loader, I/O buffers,
   address metadata, and any overlay file descriptor or cache.
2. Define the on-disk module format and linker support or static overlay
   addresses. Existing DXR/DLINK has no established user-overlay ABI.
3. Ensure there are no live return addresses into overwritten code; support
   failures, nesting and reentry without relying on accidental call order.
4. Preserve startup failure diagnostics, cold-reboot behavior, and
   configuration-file validation; prove that the overlay can be unloaded
   before resident child-management begins.
5. Benchmark loading on slow storage, including DECtape; compare overlays
   with a separate startup helper before committing to kernel/loader work.
6. Run full boot, multi-TTY, child-exit/respawn and 32K physical-core
   regression tests. A smaller INIT alone does not guarantee DSH fits
   alongside the resident kernel and other processes.

Overlay support is a **design candidate**, not implemented functionality.
The first safe optimization requires no overlay manager.
