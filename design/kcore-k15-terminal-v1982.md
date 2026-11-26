# K15 v1982: exact terminal ownership and staged VM destruction

Parent 1a135f34. This is a staged K15 subgate, not full lifecycle
acceptance. No installed build enables DAIMOS_PBM_TERM_V1982 or
DAIMOS_PBM_VM_TERM_V1982. The old interleaved bitmap path is unchanged.

## Terminal identity

The existing u-area BRK word0407 holds floor,,current during all
returning process lifetimes. vm_space_brk_current reads it only for
an active resident APR. RUN/EXEC initialize it for a new image. Once
the nonreturning terminal finalizer owns TRANSITION and has excluded
returning executive stacks, FILE and user mappings, the heap bounds
are dead. Use this word only in the directory terminal export to
escrow exact original encoded length,,physical base. Word067 LH keeps
terminal bit31, both kind bits29/30 and original source sector;
word067 RH stays the original EXIT status. Actual PREVM instructions
preserve these bits, and do not write BRK. Both immutable backing
identities (swap record and word0406) remain unchanged; backing busy
scans continue to protect the executable. No added u-area word.
The terminal marker is incompatible with ordinary restore/recovery
and is not a deferred pin provenance token.

Reuse exact u-area/MM proof from K14; restore still requires slot zero,
terminal admits a noncurrent victim on another stack. The caller must
have excluded returning executive stacks (existing proc_finish_slot
contract); the terminal leaf separately checks no user map and empty
FILE table. Require valid slot, TRANSITION, live process state, exact
unpinned KERNEL_DYNAMIC u-area, marked backing match, mode3 and VM0.
Do not require mounted backing or readable bitmap payload. Verify
full aligned length and K13 bitmap geometry. First discovery proves
sole exact PACKED_PROCESS even if pinned, records original span before
attempting release. Retry validates terminal tag and exact saved span;
no rediscovery of changed owner, base or length. Dirty DROP/FULL or
other nonterminal markers refuse; ordinary PREVM status is preserved.

Under PI exclusion, use v1917 exact range/owner proof. Free only a
sole unpinned original extent. A successful reply without absence
is failure; lost success can commit when the exact original range is
absent and no other user owner exists. Foreign intersection, extra
owner, malformed MM table and pins remain quarantined. No decoding,
backing I/O, allocation or pin retirement. On exact absence clear
mode3, terminal-specific marker and BRK, preserving FINALIZE/PREVM
and status. Common proc_swap_detach then clears backing and executable
credential identity. TRANSITION remains until the caller finishes
VM and u-area lifecycle. Mode2 dispatches unchanged RAW v1609.

## VM gate

Default vm_space_destroy is unchanged. With explicit test-only
DAIMOS_PBM_VM_TERM_V1982 assembly symbol, retain the resident-mode
check and require zero APR for any nonresident mode; dispatch existing
common detach and selected terminal backend for mode/owner validation.
No separate mode3 scanner in fixed kernel. VM_NONE is published only
on successful detach. No installed target supplies the symbol or
selects the directory terminal export yet. In particular a successful
source-linked transaction does not establish full finalizer/stack
exclusion or scheduler safety in a running system.

## RAM and tests

Matched KCC -O PDP6 DOBJ: prior FULL1904, refactored restore-only1891,
with terminal2240 (+336 vs parent). Default RAW FULL1046 byte-identical.
Extracted original/default VM gate73 words byte-identical; staged65
(-8). Combined incremental staged MRES TEXT2516; net2508 if staged
VM gate selected. No new DATA/BSS or per-slot RAM; no installed growth.

Testkit6d684e2, evidence relative to TMPDIR:

- pure-bitmap-terminal-v1982.grVrtN: both PDP6 and KA10 each65
  counted calls through common detach/default VM refusal, then65
  through staged VM destroy. Real v1979 DROP, real MM and PI, original
  RAW terminal, common detach and original PREVM marker preservation
  and retry rearm extracted from proc_pdp6.s. Plain terminal, unmounted
  backing, malformed payload, fatal-close and noncurrent foreign
  terminal, real allocator reuse, lost/false free, pins1..3, changed
  base/length, foreign post-free interval, bad table, invalid modes,
  dirty marker and FILE refusal. Exact status/metadata and adjacent
  owner preservation. Six unsafe mutants rejected (octal2,2,14,20,20,20):
  trust reply, no span escrow, adopt current length, no sole owner
  proof, no process proof, no FILE check. No full scheduler/PI6 or
  complete finalizer execution; VFS/fault fixtures controlled.
- pure-bitmap-service-v1981.uqclD7: each46 and five mutants PASS.
- pure-bitmap-full-v1980.VohPuz: each52 and six mutants PASS.
- pure-terminal-identity-v1920.jLkPWX: prior27 RAW cases and five
  mutants PASS (before staged VM symbol change; default gate DOBJ
  byte-identical). Deliberate unsafe mutants may emit expected notes.
- k15-term-v1982.KrwgYl: matched FULL and VM object counts; default
  objects byte-identical. New candidate compiled without warnings.

K15 remains open: source-built module link/selection, actual scheduler,
PI6 and RUN/EXEC/EXIT/WAIT/finalizer with returning-stack exclusion.
K16 retains deferred dirty/uncertain recovery and physical/net RAM,
stack and interrupt latency acceptance. The terminal escrow relies on
intact kernel metadata; it is not a checksum or an allocation generation.
Default mode3 and RAW PURE remain OFF.
