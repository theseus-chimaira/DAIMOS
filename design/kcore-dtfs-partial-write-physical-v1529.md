# KCORE v1529: real interrupted DTC3 block and sticky quarantine

Date: 2026-11-13. KCORE feature-completion scope only.
VERSION 0.5, strict 32K placement HOLD, no optimization.

## Genuine mixed-old/new physical block

The complete booted 256K KCC PDP-6 guest runs the
existing v1527 KCORE and installed native DTFS/TSFS
MRES from physical DTC0-2. Real PID1 mounts a newly
stamped 72-bit DTVOL1 native DTC3 volume read/write,
creates /MOUNT/DT0/PARTIAL, extends the file to two
words and writes the initial values through real
FILE/VFS/DTFS and Type-551/Type-136 physical I/O.

At a synchronized READY_ARM checkpoint, the testkit
pauses the simulator, detaches DTC3 to flush the real
18-bit-lane DECtape image, and verifies both original
36-bit words at RBN0145 on disk. It preserves a full
preimage, reattaches the same stamped tape, and arms
the versioned simulator's **test-only** one-unit,
one-block fault:

    SET DTC3 PARTIALWRITE=0145:1

Under compile-time PDP6_DTC_PARTIAL_WRITE_V1529 only,
the simulator's actual Type-551 FNC_WRIT transfers
count *changed* physical 36-bit words in both
directions. After the first changed word, subsequent
write service interrupts with DTB_ILL and remains
faulted across controller retries; it does not make
the remainder of the block look committed.
Normal SIMH builds have no injection handler.

The original open FILE seeks back to offset zero
and attempts to replace both data words. The
production monitor, device driver and DTFS provider
return failure, and the simulator records:

    DTC3: V1529 PARTIALWRITE block 0145 after 1 changed word (2 physical words)

At READY_VERIFY the testkit physically detaches
DTC3 again to flush the partially modified image.
Its independent verifier compares **all 128** physical
36-bit words against the preimage: the RBN header
and all unrelated words are unchanged; exactly
one requested data word holds its new value; the
other still holds its old value. The accepted
host evidence is:

    PASS v1529: two seeded words on real DTC3 RBN0145
    PASS v1529: physical RBN0145 changed only data slot 1; the other requested new word remains old

After reattaching the **same damaged** 72-bit-ID tape,
KCORE continues refusing FSINFO and pathname STAT
on the original mount. Repeated CLOSE refuses and
keeps the FILE owner; ordinary UNMOUNT refuses
instead of recycling the mount ID. Independent TSFS
root FSINFO remains provider 7. The booted PID1
prints DTFSPART V1529 STICKY_CONFIRMED and PASS;
the full guest runner exits zero.

Unlike v1528 (detached media *before* failed write),
this is a physical DECtape block containing
simultaneously old and new data, proven from the
actual flushed simulator tape file. No filesystem
or device response was mocked in the guest. It
validates KCORE's existing WRITE_UNCERTAIN policy
against the concrete corruption it was designed
to contain.

## Simulator build provenance and controls

Early experimental diagnostic simulator builds
replaced the complete SIMH CC variable with a
short gcc command, silently removing default
platform flags. Even the previously passing
v1528 one-word physical control then failed.
Those runs were rejected as simulator-build
artifacts, NOT treated as KCORE defects.

The accepted pdp6-v1529r5 test simulator was
rebuilt with the full output of SIMH's default
CC configuration, plus only the compile-time
v1529 diagnostic macro, with normal -Werror and
register-sanity checks. It was installed via
a versioned local.tar.gz under PDP10_PREFIX.
The original v1528 one-word WRITE_UNCERTAIN
full-guest control PASSed on this same simulator.
Native source-linked empty-file one/two-word
resize and independent physical Type-551 RBN0145
write/read also PASSed.

An overlapping attempt ran two full guests
against the same mutable prebuilt DTC3 tape,
so both were stopped and their results were
discarded. The accepted v1529 full guest used
an isolated versioned 39 MB copy of the exact
same full KCC image, including rewritten
absolute boot-media paths. Thus no concurrently
running test could replace its DTC3 image.

The independent v1527 physical different-volume
refusal/same-volume return regression also PASSed
under the complete-flags v1529 simulator with
a second isolated root image. Native chain-walk,
resize and empty-file growth tests PASSed.

## RAM and remaining feature gate

**Zero production changes:** no KCORE assembly/C
change, no optional MRES code or BSS growth, no
new resident table, userland runtime service,
on-media field or format change. Optional
DTFS/TSFS MRES remains octal 003747 text,
000002 BSS. The simulator-only fault handler
is behind a diagnostic preprocessor condition.

P0-B physical READ_SUSPECT, changed-volume
refusal, same-volume read return, failed-write
quarantine, and now genuine partially completed
physical WRITE with sticky WRITE_UNCERTAIN are
accepted. **P0-B checked administrative
recovery/discard remains OPEN**: never clear
WRITE_UNCERTAIN merely on an identity match,
never abandon FILE/cwd owners or recycle a
still-referenced VFS mount slot. P0-A permanent
critical root/BACKSTORE loss, P1-C mixed-provider
faults, P1-D dynamic FSINFO capacity, P1-E
swapless PURE/MM/APR and feature freeze remain
unchanged.
