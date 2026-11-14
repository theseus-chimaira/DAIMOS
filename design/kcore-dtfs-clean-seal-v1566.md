# KCORE P0-B v1566: owner-exclusive physical DTVOL2 CLEAN transaction

Date: 2026-11-14. **Native and real Type-551 physical CLEAN PASS.**
P0-B remains OPEN, and the v1555 production DTVOL2 read-only veto
remains mandatory and unchanged.

## Implementation

After v1562 quiescence acquisition, provider SYNC, and the final
VFS FILE/cwd, PIPE, process-swap/PURE and MM owner scans, an
INTENT_ARMED V2 mount now invokes private DTFS MRES operation 027
while keeping the v1562 exclusive admission bit. Unarmed RO/CLEAN
mounts take the previous zero-write retirement path unchanged.

`dtfs_intent_mount_seal_v1566.s` authenticates the exact published
root vnode, zero-based slot/public mount ID, native DTVOL2 flag,
held QUIESCING bit, previously verified INTENT_ARMED bit, no sticky
I/O fault bits, *writable* VFS policy, empty retained MM scratch,
and both nonzero saved 36-bit identity halves. It calls the shared
v1552 MM lease via the new `dtfs_intent_owned_clean_v1566` entry,
allocating and pinning the same 0200-word dynamic scratch rather
than introducing any fixed block buffer.

Once pinned, the wrapper marks the actual mounted physical unit
WRITE_UNCERTAIN **before** raw Type-551 I/O can start. The new
`dtfs_intent_clean_v1566.s` reads physical RBN0 into wiped scratch,
requires full DTVOL2/WRITE_INTENT and the original exact 72-bit ID,
changes only state to CLEAN and the independently salted seal word,
invalidates BCACHE, writes complete physical RBN0, wipes scratch
again and physically rereads, validating a complete CLEAN record
and both saved IDs. Unrecognized, incomplete, stale or changed
media can never be promoted from DIRTY to CLEAN by this routine.

The shared MM lease then unpins and frees the exact owned extent;
cleanup failure preserves the exact retained `aux[slot]` owner.
Only after successful physical CLEAN readback and MM release does
the mounted provider revalidate original root, media word plus
expected WR_UNCERT, saved IDs and still-writable policy, clear
WR_UNCERT and INTENT_ARMED, and report success. It never clears
QUIESCING. VFS independently checks media fault/ARMED flags again,
rescans all mount owners and MM aux **after** the potentially sleeping
physical transaction, then releases QUIESCING and retires without
another sleeping step. A possibly partial physical CLEAN write
retains the published mount, WR_UNCERT and QUIESCING; a failure
before physical I/O may return without poisoning and can retry.

The central v1555 RO guard, ordinary DTFS mount admission and the
DATA/DIR write veto are unchanged. Therefore no production V2 RW
volume can reach INTENT_ARMED and no physical CLEAN is performed on
normal production RO unmount. V1566 closes the checked CLEAN protocol
prerequisite without enabling writable production media.

## Evidence and scope

- 18 actual PDP-6 raw CLEAN and host-generated real RBN0 cases, 11
  unsafe native source mutations rejected.
- 37 actual PDP-6 mounted-seal owner, policy and MM cases, 10 mutants.
- 11 actual shared pinned-MM CLEAN wrapper cases, seven mutants.
- 18 source-linked resident VFS unmount/sleep/rollback cases, six
  unsafe mutations. **84 cases, 34 rejected source mutants total.**
- Prior v1552 pinned-MM and real-MM, v1554 verify-MM, v1555 RO mount
  and write guard, v1556 recovery, v1561 intent, v1562 unmount and
  v1563 executable owner regressions PASS after integration.
- Full 256K production KCC KCORE and optional DTFS MRES image PASS.
  `__kcore_low_end` moves from octal `024330` to `024364`, +28 fixed
  TEXT words and **zero additional fixed DATA/BSS words**.
- The actual full Type-551 PDP-6 DTVOL2/CLEAN RO guest and a second
  DTVOL2/WRITE_INTENT refusal cold boot still PASS, including full
  pre/post physical image SHA-256 equality.
- A source-matched DISPOSABLE full KCC PDP-6 guest additionally
  uses a test-only private MRES op 030 to physically arm DTC3 DIRTY
  using the existing production v1551/v1552 writer, then temporarily
  permits this one test mount to be cleaned by the unmodified v1566
  VFS/MRES final transaction. The source-matched copied test image
  proves two real DIRTY->CLEAN Type-551 cycles and whole-tape SHA-256
  equality. No test-only arm or RW relaxation is committed to DAIMOS.

Reproduce and fixture details:
`daimos-testkit/tests/validation/daimos-dtfs-clean-v1566.md`.

## Remaining P0-B

The physical test above uses a controlled test-only RW transition;
it does NOT prove safe production RW admission. In particular the
Type-551 controller still lacks a verified media-generation/exchange
contract between a checked RBN0 and subsequent transfers. Need
real interrupted physical DATA/DIR and CLEAN-prefix writes with
return/reinsertion, replacement identity and second cold boot;
closed-state owner/PIPE/swap lifetimes under those faults; and full
guest cutover acceptance before ever removing v1555 READ-ONLY.

P0-A closed; **P0-B OPEN**, P1-C/D/E unchanged. VERSION 0.5.
Strict 32K and general optimization remain HOLD.
