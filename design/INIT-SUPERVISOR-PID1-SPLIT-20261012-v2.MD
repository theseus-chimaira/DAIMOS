# Two-stage PID 1: INIT then resident SUPERVISOR (2026-10-12 v2)

## Design

DAIMOS keeps **PID 1** throughout startup. The kernel initially launches
`/SYSTEM/INIT` as PID 1. INIT reads `/CONFIG/SYSTEM`, establishes storage
policy, starts and waits for MOUNTALL, parses `/CONFIG/INITTAB`, then
performs a transactional `SYS_EXEC` to `/SYSTEM/EXEC/SUPERVISOR`.
An EXEC replaces the image in the same process, **not** a fork/launch;
the permanent SUPERVISOR therefore remains PID 1. Startup-only parser,
configuration and mounting code are not in its resident image.

## Bounded INITTAB handoff

The current EXEC V1 ABI accepts at most 16 argument records. INIT passes
exactly two validated SIXBIT records per service for up to eight services:

- a four-character record encoding the two-digit TTY number (00..20),
  action (`R` respawn / `O` once), and output sink (`D` display / `N` native);
- the original counted SIXBIT executable pathname, unmodified.

SUPERVISOR verifies the PID, even nonzero argument count, entry widths,
TTY ranges and policy values before starting children. It owns the complete
eight-entry table, tracks child PIDs, reaps exited processes and respawns
configured services. It does not re-open or re-parse INITTAB. A failed
transactional EXEC leaves INIT able to report an explicit error, rather
than accidentally becoming a second PID or losing the service table.

This format consumes the full EXEC argument-count budget at eight entries.
It is not an invitation to reduce INITTAB capacity or path lengths.

## Measured text and BSS

PDP-6 KCC/DAS/DLINK, octal 36-bit words:

| Executable | Image | BSS |
| --- | ---: | ---: |
| Original combined INIT | 005374 | 000536 |
| Startup-only INIT | **004210** | **000461** |
| Resident SUPERVISOR | **001530** | **000461** |

The startup image has a temporary VM extent of 010000 octal words.
SUPERVISOR's image plus BSS plus 002020 origin/stack totals 004231 octal
words, rounded to **006000**. The persistent PID 1 VM thus saves **02000
octal (1024 decimal) words** compared with the 010000-word startup extent.

The libc work also splits the S6REC reader out of the ordinary text
reader and gives INIT/SUPERVISOR a tiny direct console writer. Regular
programs retain the full S6REC and buffered-output implementation. DSH
remains under its normal 036000 image limit (035727 on this build).

## Overlay research

Separate resident and startup executables achieve the essential overlay
effect through the existing transactional EXEC mechanism, without an
overlay-aware linker or a kernel change. Further overlays are an optional
research track, **not implemented**. Any overlay manager would need
measured net memory savings, safe cross-overlay returns, relocations,
loading-error semantics, and performance analysis on slow DECtape/disk.
With startup code no longer resident, overlays in the remaining compact
SUPERVISOR are unlikely to repay their loader complexity.

## Acceptance requirements

Before claiming a completed boot transition: fresh PDP-6 disk image,
PID 1 INIT-to-SUPERVISOR handoff, LOGIN/DSH launch, standard DSH suite,
multi-entry ONCE/RESPAWN behavior, and a dedicated 32K boot. In
particular, a successful 256K boot does **not** establish that transactional
EXEC can stage the replacement image under 32K memory pressure.
