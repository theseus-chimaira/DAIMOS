# KCORE P0-B v1570: checked DTVOL2 poisoned-media replacement

Date: 2026-11-14. **Native and full physical PDP-6 acceptance PASS.**
Production DTVOL2 remains READ-ONLY; P0-B remains OPEN.

## Small production change: exact-format replacement verifier

The original `dtfs_verify_replacement_v1531` private provider op
026 admitted only a DIFFERENT, physically valid native DTVOL1
replacement of a WRITE_UNCERTAIN mounted DTFS. It could not retire
an otherwise valid poisoned V2 owner after a different V2 reel
was installed. This was an actual missing P0-B recovery feature.

The v1570 extension branches on the **fresh, uncached physical
RBN0 magic and the retained original VFS media FORMAT_V2 bit**:

- Poisoned V1 -> different complete valid V1 only, unchanged v1531
  original checks and no cross-format upgrade.
- Poisoned V2 -> different complete *CLEAN* V2 only, using the
  production `dtfs_intent_validate_v1550` full magic, 72-bit ID,
  complement, exact state, salted seal and zero-padding parser.
- V2/DIRTY, corrupt/malformed V2, uninitialized/short-successful
  RBN0, legacy zero media, V2-to-V1 downgrade, identical 72-bit ID
  and invalid native RBN0144 directory are refused.

The original v1531 physical directory/raw RBN0 read, shared cache
invalidation, per-provider I/O lease, two-phase VFS owner scan,
MM-pinned-scratch empty check, privileged explicit DROP_FD and
checked old-slot retirement are unchanged. The old damaged medium
is never written, repaired or automatically marked healthy. New
V2 mounts must still pass v1555's production READ-ONLY admission.
No new permanent KCORE DATA/BSS, mount slab word or boot ABI.

## Native source and unsafe-mutation acceptance

Thirty source-linked actual PDP-6 cases cover independent full
V1/V2 physical images, same/different high/low ID, V2 CLEAN vs
DIRTY, bad seal/identity/padding/state, V1/V2 cross-format and
false-successful/partial raw Type-551 reads, and all original
native RBN0144 directory invariants. Eight unsafe mutations are
rejected; the old 20-case/six-mutant v1558 physical replacement
preflight was rerun and PASSed with the new parser linked. The
original complete physical V1 v1531 admin replacement guest also
PASSed against the v1570 production build.

## Actual PDP-6/Type-551 live replacement

An isolated source-matched full KCC guest enabled exact DTC3/mount2
V2 writes for test purposes only, after first proving that the
production v1555 RW mount is refused. Genuine v1551-v1554
first-write/MM verification physically stamped DTVOL2/DIRTY and
wrote two file words. Diagnostic Type-551 `PARTIALWRITE=0145:1`
interrupted the next DATA write after one changed 36-bit word.
Independent DETACH/full-tape comparison verified the other word
was still old and the full valid DIRTY record was preserved.

The original mounted guest refused ordinary CLOSE and automatic
UNMOUNT. Privileged replacement refused with FILE and cwd still
owned; only exact DROP_FD and cwd release removed those owners.
Reinserting the **same original damaged reel** still refused
retirement. Physically detaching it and attaching a new,
independently stamped, different-72-bit-ID V2/CLEAN reel then
allowed the unchanged v1531 VFS owner recheck to retire the old
slot. The new reel mounted RO, rejected CREATE, did not contain
the old file and unmounted normally. The guest reached PASS.

Each SIMH physical reel used a distinct backing file. Corrected
whole-image comparisons proved neither the old detached DIRTY nor
the new attached CLEAN medium was changed by administrative
recovery. The original damaged tape's SHA-256 is
`e06664a3bac2dc0a6c730648871eeb0692b22cdde19aec82b65c336b1c59aa94`;
the new CLEAN reel's is
`2a32fdea8e863f3153fd5ce2d83ae8c3887639d6f81755339c90b4891d1ffbe4`.

Two **separate cold boots on unchanged production KCORE/MRES**
then completed: original DIRTY tape refused RO/RW and published
no slot; the replacement CLEAN tape mounted RO and completed the
owner/unmount/remount guest. Both whole-tape SHA-256 values were
unchanged. The old v1531 actual V1 administrative guest PASSed.

Testkit exact source/limitations:
`tests/validation/daimos-dtfs-v2-admin-replace-v1570.md`.

## Release boundary / RAM

The hardware Type-551/136 interface provides no reliable
reel-generation counter, prevent-eject interlock or atomic
identity check bound to a following physical WRITE. An operator
can change external media between separate controller transfers;
repeated RBN0 checks or volatile software epochs cannot solve
this TOCTOU. v1570 explicitly handles *known poisoned* mount
replacement after ownership release, not arbitrary online
exchange of writable media. **The v1555 production read-only
veto remains mandatory and P0-B stays OPEN** until a defensible
hardware/operating media retention policy is specified and proven.

Only optional DTFS MRES TEXT grows slightly. Fixed KCORE low end
remains `024364` octal, with zero permanent KCORE DATA/BSS and
zero mount-table slab growth. P0-A CLOSED, P1-C/D/E unchanged,
VERSION 0.5, 32K optimization HOLD.
