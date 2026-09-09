# DAIMOS KINIT self-reclaim census

Date: 2026-09-08

## Base

This pass is based on DAIMOS tree:

`e4c1b9d534e205cf1bd6d35afd1ae7842f9c1afb`

That tree contains the completed UUO ABI, process/MM packing, structural-sharing,
compaction, swap, whole-kernel, expendable-init, and compact module-metadata
passes.

## Goal

Remove the remaining first-user bootstrap from permanent KCORE without leaving
a resident trampoline or permanently reserved disposal code.

## Design

The late bootstrap is linked into a small protected text hole inside KINIT.
Before loading INIT it publishes the two KINIT ranges surrounding that hole to
MM.  This leaves only the instructions which are still executing unavailable
to allocation.

After INIT, its process extent, and its stable u-area are fully constructed,
the late bootstrap publishes its own text hole with `mm_add_free()`.  On the
PDP-6 this does not alter physical memory; it only updates the MM extent table.
Priority interrupts are still disabled and the terminal path performs no
allocation, compaction, module movement, or swap operation.  The physically
unchanged instructions therefore execute safely until `mach_enter_user()`
transfers control to INIT.

There is no permanent KCORE trampoline and no permanently reserved bootstrap
extent after user entry.

The same disposable hole also owns all code now proven to be used only before
first user entry:

- `exec_load_init()`;
- `proc_slot_claim()`;
- `proc_user_context_init()`;
- user-UUO trap installation;
- the final KINIT-to-INIT handoff itself.

## Resident measurement

Fresh same-toolchain PDP-6 links:

| Metric | Previous tree | Self-reclaim tree | Saving |
| --- | ---: | ---: | ---: |
| `__kcore_low_init_end` | `014320` | `013536` | `000562` / 370 words |
| `__kcore_low_end` | `015054` | `014272` | `000562` / 370 words |
| normal `MEMSTAT RESIDENT` | 11,072 | 10,702 | 370 words |

The protected late KINIT hole in the measured production build is:

- begin: `054137`;
- end: `054764`;
- size: `000625` / 405 transient words.

It is transient only and is returned to MM before first user execution.

## Correctness constraints

The self-free is safe only while all of the following remain true:

1. the late text hole is excluded from both earlier `mm_add_free()` calls;
2. the stack has already moved to the pinned permanent kernel stack;
3. INIT and its u-area are completely allocated and initialized before the
   final self-publication;
4. priority interrupts remain disabled;
5. after the final `mm_add_free(late_base, late_words)` there is no allocator,
   compaction, module movement, swap, or other core-reuse operation before
   `mach_enter_user()`;
6. `mm_add_free()` remains descriptor-only and does not clear or otherwise
   overwrite the supplied physical range.

The low-memory regression now checks that the old permanent boot-transition
symbols are absent from the KCORE map and that the KINIT self-reclaim marker
ordering exists.

## Validation

Green on this tree:

- host `mm-v1` including movement and swap regressions;
- real five-process PDP-6 scheduler/preemption/RL-PL regression;
- fresh 32K protected-user boot/EXIT;
- fresh 64K protected-user boot/EXIT;
- 96K RAMFS threshold boot;
- fresh 256K protected-user boot/EXIT;
- DAS negative relocatable-addend regression;
- direct monitor-UUO encoding regression;
- final KCORE DXR2 validation with `dxrcheck`.

The measured 32K `MEMSTAT` result is:

```
TOTAL 32768
RESIDENT 10702
PROCESS-WORDS 5416
RAMFS-CAPACITY 0
PROC-SLOTS 2
PROC-SLOTS-MAX 64
```
