# P1-D v1591: alternate physical TSFS root inventory with eight slots

Date: 2026-11-15. Physical 256K KCC PDP-6, unchanged production
KCORE/FSINFO/MM/VFS and DTFS/TSFS MRES. This closes the explicitly
missing **above-seven-slot physical inventory** slice of P1-D.

The opt-in v1591 testkit builds a two-member TSFS root using original
`mktsfs`, DTC0 and DTC1 Type-551 physical media, and an **empty**
DTC2 transport. It reuses a full KCC boot image only as a read-only
input. Original TSFS discovery records the two exact physical root
members; KINIT's installed inventory counts the root as one mount,
six spare Type-551 transport slots and one available MEMFS provider.
The real FSINFO capacity is **8**, not the previous three-member
root profile's 7. No staged synthetic value is written to VFS.

The four independent DTVOL1 72-bit-stamped DTFS tapes on DTC3..6
physically mount as IDs 2..5 with the original provider and pathname
records. Every empty upper index 5..7 returns zero; 8 and 9 return
invalid. A middle unmount leaves original high mount ID 5 and its
path unchanged. All remaining nonroot mounts retire and slot 0
continues to report TSFS provider 7, ID 1 and capacity 8.

The full guest exited zero and emitted:

```
FSINFOINV V1591 START
FSINFOINV V1591 CAPACITY8
FSINFOINV V1591 MOUNTED5
FSINFOINV V1591 HOLE_STABLE
FSINFOINV V1591 RETIRED
FSINFOINV V1591 PASS
```

The initial test-only C file had one unused local and its KCC
diagnostic correctly identified it. The local was removed; a
fresh target KCC source-to-assembly compile completed warning-free.
No assembler/kernel correctness bug was encountered. Physical
testkit report: `daimos-testkit/tests/validation/
daimos-fsinfo-two-member-root-physical-v1591.md`.

Acceptance does NOT prove the 26-slot representational ceiling is
a physical machine configuration. P1-D remains open for actual
DSK/DRM-root hardware/service inventories and provider failure
semantics. No production code, RAM, ABI, media format or KCORE
placement changes. P1-E is separately paused for cross-target
KA10 architectural planning; do not implicitly integrate the
historical PDP-6-specific swapless-PURE design. VERSION 0.5,
32K/optimization HOLD.
