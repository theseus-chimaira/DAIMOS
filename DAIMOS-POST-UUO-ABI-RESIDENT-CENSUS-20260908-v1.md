# DAIMOS post-UUO ABI resident census

Date: 2026-09-08

Baseline tree: `fb3fe257283e8f2062eb1dba9ac73c532584ef31`

This census is taken after the direct monitor-UUO syscall ABI cleanup and
per-process syscall-context compaction.  Normal MRES modules are unchanged from
the post-scheduler census.

## Fixed KCORE

Fresh baseline/current builds with the same DAS/dlink toolchain report:

| Item | Baseline | Current | Delta |
| --- | ---: | ---: | ---: |
| text | `014636` / 6,558 | `014631` / 6,553 | -5 |
| data | `000003` / 3 | `000003` / 3 | 0 |
| BSS | `000564` / 372 | `000560` / 368 | -4 |
| fixed KCORE total | `015425` / 6,933 | `015414` / 6,924 | -9 |

`dxrcheck` current result: `image=014634 bss=000560`.

The five text words are the net result after replacing the generic syscall
number path with direct monitor-UUO decoding; the four BSS words are the removed
AC2..AC5 syscall shadows.

## Userspace syscall veneers

Fresh DSH links with the same compiler/toolchain report:

- baseline DSH image: `006343` / 3,299 words;
- current DSH image: `005636` / 2,974 words;
- reduction: `000505` / 325 words;
- DSH BSS remains `000510` / 328 words.

The shared assembler veneer object replaces separately emitted static C wrappers
in each DSH translation unit.  The remaining direct veneer set is 56 words.

The process user extent remains `012000` / 5,120 words because both old and new
DSH images fall in the same PDP-6 `02000`-word allocation bucket after origin,
BSS, and user-stack reservation are included.

## Per-process executive u-area

The syscall shadows were also present in every sleeping kernel context.  After
removing them, the now-dead gap before cwd/file state was closed while retaining
exactly the old private kernel-stack capacity.

- old u-area: `000450` / 296 words;
- new u-area: `000435` / 285 words;
- saving: `000013` / 11 words per active process;
- private kernel stack remains `000320` / 208 words.

Per resident protected DSH process is now:

- user extent: `012000` / 5,120 words;
- stable u-area: `000435` / 285 words;
- total: `012435` / 5,405 words.

## Normal loaded modules

Unchanged from the post-scheduler census:

- module image text+data: 3,119 words;
- module BSS: 50 words;
- retained relocation maps: 179 words;
- total normal MRES extents: 3,348 words.

No movable-module relocation functionality was removed.

## Normal kernel resident low set

Reconciled fixed resident set:

- low core below `000060`: 48 words;
- fixed KCORE: 6,924 words;
- pinned permanent kernel stack: 1,024 words;
- normal MRES extents: 3,348 words;
- total: `026120` / 11,344 words.

This is 9 words below the post-scheduler value of 11,353 words.

## Boot-time KINIT impact

Fresh baseline/current KINIT links report:

- baseline DXR image: `037301`;
- current DXR image: `037274`;
- image reduction: 5 words;
- BSS remains `001274`.

The change is from the smaller embedded KCORE image; no KINIT policy change is
part of this pass.

## Projected free core after one protected DSH boot

Only fixed KCORE (-9) and one u-area (-11) affect the previous occupied totals;
process-table sizing and RAMFS policy are unchanged in this ABI pass.

| Core | Occupied | Free |
| --- | ---: | ---: |
| 32K | `041455` / 17,197 | `036323` / 15,571 |
| 64K | `042355` / 17,645 | `135423` / 47,891 |
| 96K | `054155` / 22,637 | `223623` / 75,667 |
| 256K | `054155` / 22,637 | `723623` / 239,507 |

These are 20 words better than the post-scheduler census at every core size.

## ABI details retained by this census

- Direct monitor UUOs use opcodes `040..073`; `074..077` remain reserved.
- UUO effective address carries 18-bit arg0 for pointers/small scalars.
- `NICE` explicitly sign-extends its 18-bit effective-address argument.
- Real arguments otherwise remain in AC1..AC4 and result returns in AC1.
- The dispatcher uses AC5 as its table index; AC0 cannot index on PDP-6 because
  index field zero means no indexing.
- `sys_user_words` uses cached PDP-6 APR relocation/protection state.

## Gates

Green after the final context compaction:

- host `mm-v1`;
- real five-process PDP-6 scheduler regression;
- 32K/64K/96K/256K low-memory protected-user boot/EXIT and RAMFS threshold;
- DAS direct-UUO encoding regression.

The next optimization pass may now begin with process/MM representation and
table policy.  The first measured target is the 96K process-slot policy: keeping
96K at 128 slots instead of 256 would recover 896 dynamic metadata words without
changing the architecture-wide 8-bit slot/PID representation.
