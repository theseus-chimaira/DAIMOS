# KCORE v1605: PDP-6 / KA10 / KI10 source portability plan

Date: 2026-11-15
Status: source/build architecture audit; PDP-6-only binary-identical
source-map integration. No KA10 or KI10 runtime implementation.
P1-E RAW PURE integration is still separately OPEN.
Current priority and scope are superseded by
`design/kcore-pdp6-first-cpu-contract-v1606.md`: complete PDP-6;
the hardware notes below are NOT an implementation roadmap.

## Accepted design constraint

Make future machine ports replace the minimum necessary code without
enlarging the current PDP-6 KCORE, any hot process path, or per-process
RAM. Select architecture code while building the image, not through a
runtime machine-vtable, extra procedure call, or kernel configuration
object. Preserve three-word process descriptors and tested user ABI
until a target truly needs a different backend-private representation.
Keep original PDP-6 symbols and source paths until testkit's
source-extraction and mutation fixtures can be migrated safely.

## Hardware differences that determine the boundary

* PDP-6: one contiguous user APR image (02000-octal/1024-word
  relocation granularity). All process VM images are 18-bit physical.
* KA10: contiguous user relocation, optional KT10A independently
  relocated/protected high segment. Start with a single low segment
  and verify actual installed relocation hardware before high-segment
  sharing. Shared high segment owners are not process-private suffixes.
* KI10: 512-word pages, 18-bit virtual and 22-bit physical addresses.
  A page's physical frame need not be next to its logical neighbor;
  page map, fast AC blocks, kernel/supervisor mode and page-failure
  handling differ from the PDP-6 context/PI/APR path. The first
  milestone is resident-only mapping (all needed pages in core),
  not demand paging or copy-on-write. Independently prove executive
  page access and the ability to copy data from frames not directly
  expressible as a plain 18-bit executive pointer.

DEC hardware reference: DEC-10-XSRMA-D (1976), sections 2.16/2.17:
https://www.bitsavers.org/pdf/stanford/sail/SAILON-75A_DECsystem-1020_Hardware_Manual_DEC-10-XSRMA-D_Jan77.pdf
Also DECsystem-10 Monitor Calls Manual (1974), section 1.2.2.
Processor details are **design input**, not proof of a DAIMOS KI10
simulator or boot support.

## Replaceable machine owners, now explicit at build time

`system/kernel/arch/pdp6_source_map_v1605.mk` is included by BOTH
`system/kernel/Makefile` and `system/boot/pdp6/Makefile`. It
identifies which source files provide:

* CPU process save/restore, PI6 software requests: `proc_pdp6.s`
* UUO/privilege entry and return: `mach_user_pdp6.s`
* PI low-core vectors, trap state and save-cell aliases:
  `kcore_pi_pdp6.s`
* syscall dispatch glue: `syscall_dispatch_pdp6.s`
* single-image VM: `vm_pdp6.s`, `vm_pdp6_swap.c`
* contiguous MM, swap, PURE backing: `mm_pdp6.s`,
  `proc_swap_pdp6.s`, `proc_pure_backing_pdp6.s`
* initial low memory/EXEC, KINIT, module relocation and resident
  support: `exec_lowmem_pdp6.s`, `kinit_io_pdp6.s`,
  `kinit_late_begin_pdp6.s`, `module_runtime_pdp6.s`,
  `bcache_pdp6.s` and `kfmt_pdp6.s`.

Do NOT interpret this manifest as a list of wholly machine-specific
policy files. Some source files still mix generic policy and hardware
details. The PDP-6 boot Makefile deliberately keeps its original
generated filenames, assembly ordering, object order, fold plan and
`kcore_end` last. A future processor needs a separate boot manifest
and linker acceptance, not merely `VM_BACKEND=ki10`.

## Keep unchanged; no speculative generic layer

* Generic process lifecycle, wait, event, clock accounting, priority,
  file ownership and user ABI; no new dispatcher on PDP-6.
* Two-word sorted contiguous MM descriptor and existing MM allocator
  for physical **resident kernel** and device allocations. For KI10
  add a separate page-frame manager rather than widening every PDP-6
  descriptor or requiring a 512-word KCORE allocation.
* Existing `proc_storage_runtime_v30.s` compact storage-state map,
  mutable RAM codec, raw disk/drum swap and P1-E lifecycle policy.
* Generic VFS/file providers and module ownership policies. Drivers
  or their I/O PI bindings may need separate hardware adaptation;
  a PDP-6 controller-specific MRES is not automatically KI10-ready.

## Source files to split at the actual CPU port boundary

1. `proc_pdp6.s` (4243 source lines at v1605): isolate
   context save, executive/user return, PI6 requests and VM activation
   from scheduling, TTY, event, WAIT/EXIT and retry policy. Preserve
   policy source and current hot inlined PI operations; do not insert
   CALL/RETURN just for indirection. ~195 testkit files refer to this
   original path; stage any move with generated assembly and native
   full-guest equality gates.
2. `proc_swap_pdp6.s` (~2167 lines): leave pressure and victim
   policy common, replace only contiguous whole-image transfer,
   relocation and resident publication. The existing source-sensitive
   testkit has ~94 references. This split should be part of real
   KA10/KI10 transfer work, not a speculative early rewrite.
3. `exec_load.s`: retain DXR validation, admission and rollback
   policy. Isolate physical compressed destination (`base+020`)
   and raw-MEMFS-XIP pinned APR publication. KI10 must load/decode
   through page-aware bounded copies; no direct physical pointer
   arithmetic. The old common-decoder prototype grew 180 TEXT
   words in an isolated comparison: do not relink it blindly.
4. `syscall_dispatch_pdp6.s` is largely generic dispatch and
   already calls `vm_user_words` for many pointers. Prefer to
   preserve the shared syscall dispatch and change only the user
   memory transfer/validation interface for paged/split ranges.
   Native `mach_user_pdp6.s` owns the hardware UUO path.
5. `syscall_info.s` COMM startup record access and
   `monitorfs_runtime.s`, `monitorfs_swap_v63.c` direct VM
   address/one-image reads need a VM-backed user-word reader.
   This is important even if the initial KI10 has NO demand paging.
6. `proc_pure_backing_pdp6.s`, `proc_xip_policy_v1394.c`,
   `memfs_data_pdp6.s`, `exec_image_common_v1386.s` and
   staged P1-E MM rebase/expand are tightly coupled to the
   PDP-6 contiguous image and XIP/source ownership. A future
   segmented/paged backend must manage shared code and mutable
   private state independently. Keep v1604 PURE lifecycle policy
   common and current PDP-6 zero-copy rebase local to the backend.

Keep complete `vm_pdp6.s` and `kcore_pi_pdp6.s` replaceable as
whole files NOW. Splitting their internals would needlessly disturb
~54 VM source-sensitive tests and the carefully aliased PI save
cells. `mm_pdp6.s` is also best left as one contiguous allocator.

## Historical KI10 compatibility issues (NOT scheduled work)

KI10 hardware differs in low-core/executive mode, fast AC and PI
handling, 512-word user page maps, wider physical frames and
page-bounded logical transfers. User buffer reads, DXR loading,
MonitorFS and PURE ownership must not rely on a PDP-6 contiguous
physical pointer. These facts justify existing *source boundaries*;
they are not an order of work, a release target, or a proposal to
introduce paging, page failure handling, demand paging or COW now.

## v1605 acceptance and no-growth constraint

An unmodified KCC full PDP-6 ROOT=tape baseline KCORE was compiled
in a separate directory, followed by this source-map build. The
complete `kcore.dxr` and `kcore.map` compare byte-identical.
Initial source-map-only generated VM, EXEC and process assembly were
also byte-identical. Final comment-only annotations alter the literal
generated EXEC/process assembly text, but a fresh linked DXR/map
still compare byte-identical. This proves no current KCORE
TEXT/DATA/BSS/image change from the refactoring.
No KI10 emulator boot or KA10 implementation is claimed.

All future staged native tests belong in daimos-testkit, not in the
DAIMOS source tree. Preserve the strict 32K/optimization HOLD and
do not incorporate dormant PURE compression into production.
