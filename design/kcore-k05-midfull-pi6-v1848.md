# K05 v1848: original PCLK PI6 inside a pinned RAW PURE FULL restore

Date: 2026-11-23. Status: **scoped physical PDP-6 PASS**.
K05-K09 remain OPEN; RAW PURE and mode3 default OFF.

## Real 48K source-linked acceptance

The exact v1845/v1847 source-built five-child PDP-6 swapless TSFS
ROOT, secondary read-only DTC3, original PURE/PROCESS ownership, native
RUN/PIPE/data/EXIT/WAIT and pressure fixture were reused without any
production code or image change. The v1848 controller verifies the
installed `proc_swap_reclaim` JRST before resolving the removable
PURE MRES FULL loader physical entry `036153`. A genuine FULL restore
for slot 3 first enters with two distinct, pinned live MM extents:

* Authoritative PACKED RAW suffix: `111200..120000` octal, pin 1.
* Temporary complete PROCESS APR: `120000..130000` octal, pin 1.

Before detaching any device, the controller enables a physical
breakpoint at permanent fixed-KCORE `proc_sched_pi_tick=017205`.
The actual 60Hz PCLK event enters this original PI6 handler with
instruction `350040024510` (`AOS 1,proc_timer_clock`). The PI6
frame's independently read return word is `340000037637`: the
interrupted executive PC is `037637`, source-linked to relocated PURE
MRES offset `005220` (within the original map interval between
`vfs_create_op=005201` and `vfs_symlink=005252`). The installed
PURE base is `032417`, derived from the real runtime reclaim JRST.

Original clock/cooldown/cursor at PI6 are octal `057171/0/3`.
The interrupted process is original slot 0, and all 18 live MM
descriptors, owners, geometry and pins at the PI6 entry are exactly
equal to those at the previous FULL loader entry: both the PACKED
source and full PROCESS destination are STILL pinned once each.
No early RESIDENT APR is advertised; the selected TSFS ROOT is
unchanged. This is an actual hardware clock interrupt **inside** a
pinned two-extent FULL transaction, not merely after cleanup.

Only after the original PI6 interrupt is witnessed does SIMH detach
the independent secondary DTC3 device. The original FULL loader then
returns -4 at its source-linked service return `033067`. The original
rollback releases only the provisional PROCESS target and unpins the
exact PACKED source. The identical private DTC3 reel is reattached;
all four secondary-backed children complete their original writable
canaries, native EXIT/WAIT and the parent finishes five WAITs and
HALT PC 0. The entire accepted v1845 physical MM/source/recovery
verifier PASSes unchanged as a prerequisite to the v1848 PI6 checks.

The stronger v1848 independent physical oracle verifies original
clock instruction, executive PI save word, source-linked interrupted
code range, PI6 slot, MM extent/pin equality and subsequent rollback.
Three separately executed forged physical traces are rejected at
their intended checks: missing saved interrupt PC, out-of-MRES saved
PC, and dropped temporary PROCESS target pin. The original normalized
physical trace SHA256 is
`6375e8dc51ef6a9e05edbb2b7c9c5252f8ad8e332e9696a05ffea0495cca195d`.

Exact source image hashes, unchanged from earlier acceptance:

* Fixed KCORE: `33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`
* Optional PURE MRES: `ad246ce14079a821cd69f4c4f7b7661f5d1a0f58c118bdfa5cfa7adf2b2bfe3f`

The testkit harness is
`tests/validation/pure-k05-midfull-pi6-v1848/`; rerun an already
captured trace using `make test-pure-k05-midfull-pi6-audit-v1848`
with `V1848_BUILD` and `V1848_WORK` as documented there. No new
fixed KCORE word, process bit, MM table, MM instrumentation or
production default was introduced.

## Still OPEN

This proves one actual PCLK PI6 event at one original pinned FULL
code point, followed by real secondary-source loss, rollback and
recovery. It does NOT prove arbitrary PI6 timing across every MM
pin/unpin/commit instruction, permanently corrupt pins or unknown
MM free returns, allocator loss/fragmentation over all owners,
continuous original physical MM write-watch high-water, KA10,
mode3, or K05-K09 production acceptance. The next K05 priority is
exact persistent allocator/target-unpin/free refusal with bystander
progress and quarantined owner policy under real multi-owner pressure.
