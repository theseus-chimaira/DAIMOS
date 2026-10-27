# DSH size reduction needed for strict 32K PDP-6 acceptance (v472)

## Distinguish two independent constraints

1. **Executable image gate:** production `DSH_IMAGE_LIMIT=036000`
   (octal) = **15,360 decimal words**. The controlled v440 KCC image
   `035757` octal = **15,343 words**, just **17 words below the cap**;
   original `036013` was 11 words above it. The shared buffer-clear
   implementation cut **28 words**. This gate is currently met by
   that measured KCC configuration; do not increase the limit.
2. **Physical 32K fit:** resident KCORE/MRES and the *allocated* DSH
   process VM, together with live INIT/other processes, stacks, MM
   metadata, alignment and fragmentation, must fit **32,768 words**.
   Meeting the DXR image gate does not satisfy this requirement.

## Source-grounded size budgets

| Basis | Resident/kernel words | DSH VM words | Combined | Over 32K |
| --- | ---: | ---: | ---: | ---: |
| Historical stripped CTY+CLK+DSK+D6FS estimate | 12,498 | 20,480 | 32,978 | **210** |
| v466-v470 full linked `PERMANENT_LAST` plus *historical* DSH VM | 15,556 | 20,480 | 36,036 | **3,268** |

The first line is the historical **best-case stripped-link estimate**
recorded in `PROJECT-STATE.MD` (48K/32K analysis): its 210-word
deficit excludes other allocations. The second line is **only a
conditional arithmetic comparison**, **not a measured current boot
requirement**: the 20,480-word DSH allocation derives from an older
DXR image `031321` and BSS `012751` plus user origin/stack, rounded
to a `02000`-word allocation boundary. Its configuration and date do
not match the current full linked kernel, nor does `PERMANENT_LAST`
represent all runtime allocations. Neither line supplies an exact
current size-cut target.

The historical 48K running workload had resident 12,725 and process
allocations 28,440, totaling 41,165 words: **8,397 above 32K**.
This is a measured *workload* comparison, not the minimal shell-boot
deficit or a target assignable entirely to DSH.

## Required reduction and acceptance method

* The only defensible **numeric floor** from the historical stripped
  configuration is **at least 210 words**, before other live costs.
  It is not an accepted engineering target for today's build.
* Rebuild **current clean** DSH with the production compiler and image
  gate; report DXR image, BSS and high-water BRK/heap separately.
  Do not use the uncommitted experimental `dsh_exec.c` heap changes as
  an accepted baseline until their lifecycle and size tests pass.
* Boot the intended 32K lowmem configuration and measure its actual
  permanent residency, INIT and DSH process allocations, 02000-word
  rounding, stack/heap peaks and MM fragmentation. The required cut is
  then `max(0, measured peak requirement - 32768)`, **plus a documented
  safety margin**; account separately for kernel and DSH savings.
* Prefer reductions in allocated VM **pages/02000-word extents** and
  permanently resident kernel words over a DXR code reduction that
  cannot lower an allocation boundary. Keep DSH features/ABI and
  filesystem semantics intact. Verify login, commands, process
  concurrency and peak-variable/function/script workloads natively.

Status: **strict 32K DSH acceptance remains deferred**. No size gate
override, production source modification or reduction is claimed by
this budget documentation.
