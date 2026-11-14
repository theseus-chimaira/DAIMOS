# KCORE P0-B v1556: physically verified DTVOL2/CLEAN read-fault recovery

Date: 2026-11-14. **Production DTFS MRES feature completed and tested**,
but the overall P0-B writable durability issue remains OPEN.

## Source sweep and closed gap

v1555 introduced real production DTFS admission for opt-in
DTVOL2/CLEAN tapes, strictly READ-ONLY. However, the existing
post-read-fault `dtfs_identity_revalidate_v1522` in the same
DTFS MRES still called only the old DTVOL1/zero parser. A
transient physical Type-551 read error set READ_SUSPECT, and
even a completely healthy original V2/CLEAN tape returning
could never pass revalidation. This was an incomplete KCORE
feature, not a reason to permit V2 writable data operations.

v1556 extends the **actual** production `dtfs_runtime.s` native
revalidation routine. With READ_SUSPECT set it invalidates the
BCACHE workspace tag, clears all 0200 shared workspace words
before the Type-551 physical RBN0 read, and records the original
MM mount slot and full media word on the process-private stack
across physical I/O. This also closes a stale-buffer hazard:
a failed or truncated physical read incorrectly returning zero
cannot inherit a previously valid on-media header left in RAM.

The routine chooses the exact validator from the *original*
media format flag, not from an arbitrary newly read magic word:

- DTVOL1/legacy: retain the original four-word/zero validator;
- FORMAT_V2: call the existing complete v1550 six-word native
  parser; require exactly CLEAN, with zero remaining padding.

Both original saved 36-bit identity halves must match the
physically read tape. The entire originally published media
word must still be identical after the sleeping operation.
V2 additionally requires the root slot still to identify the
correct DTFS provider and public mount ID, and the VFS slot
must still be READ-ONLY. Only after *all* of these checks may
the routine clear READ_SUSPECT. It does not clear the volume
format, 18-bit UID/GID mount owner, armed state or any other
state bits. WRITE_UNCERTAIN remains a sticky unconditional
refusal even when RBN0 and the saved ID match.

A physical V2 DIRTY record is never accepted by this RO
recovery mechanism, even if its identity and seal are intact.
There is no V2 RW promotion or implicit CLEAN rewrite. Raw
reads use the original physical unit and the shared provider
lease; no new KCORE slab, permanent scratch or syscall ABI.

## Executed acceptance

The new `daimos-testkit/tests/validation/
daimos-dtfs-v2-revalidate-v1556.sh` compiles the production
host `mkdtfs` and decodes RBN0 words from two independently
formatted V2 tapes and one V1 tape. Its native PDP-6 fixture
links the actual production revalidation code, original V1
parser, and unmodified v1550 V2 parser. Only Type-551 hardware
completion is simulated; its bytes come from real formatter
images. **29 native cases PASS and 14 unsafe source mutations
are rejected.** They include full CLEAN V2 recovery and
owner preservation, raw error, short or no-byte false-success
transfers, DIRTY and replaced tapes, torn seal and padding,
invalid state, changed saved IDs, WRITE_UNCERTAIN appearing
during a sleeping transfer, lost RO, wrong provider/mount,
and unchanged V1 valid/replacement handling.

The existing native `daimos-dtfs-mounted-id-v1522.sh` also
passes its complete legacy workflow and all eight unsafe
source mutants with the v1550 validator linked. The real
full-profile **KCC DTFS MRES assembly, fold and link** PASS,
with no new MRES registration or permanent KCORE data words.
The new test fixture is kept exclusively in daimos-testkit.

## Still unresolved

No writable DTVOL2 DATA/DIR transfer is enabled: the v1555
central write guard continues to refuse it. The already
independently tested v1551-v1554 physical DIRTY arm/verify
and MM-pinned scratch path still requires mandatory provider
write routing and correct retained MM scratch ownership.
The VFS unmount path still needs an exclusive owner barrier
across sleeping final CLEAN on-media seal and readback.

This native test does not constitute a full 256K PDP-6 guest
Type-551 DMA/interrupt, partial write, changed DECtape, or
cold-boot acceptance. Removable-media exchange between the
future pre-write RBN0 verification and DATA transfer remains
unresolved; provider serialization does not mechanically
prevent media removal. Do not use this feature to claim P0-B
is complete or to enable writable V2 prematurely.

Next: complete the provider MM ownership and mandatory DATA/
DIR write gate under the existing serialized filesystem lease,
then exclusive CLEAN/unmount, full physical reinsertion and
cold-boot tests. P0-A complete; P0-B OPEN; P1-C separate.
VERSION 0.5, strict 32K and optimization HOLD.
