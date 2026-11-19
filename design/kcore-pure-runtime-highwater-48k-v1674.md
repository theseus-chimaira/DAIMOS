# v1674 physical PDP-6 48K RAW PURE committed-MM high-water: PASS

Date: 2026-11-19. Scope: unmerged PURE branch
`kcore-pure-reclaim-pressure-v1673-20261118`, source-based physical
original-memory PDP-6 tests using the versioned daimos-testkit target
`test-pure-runtime-highwater-v1674`. No production KCORE or MRES was
changed by this measurement. No raw DISK/DRM swap attached; mode3 off.

## Original execution and identity

The source-built GENERAL and STRICT swapless 48K profiles booted
isolated three-member TSFS roots containing the same C89 test-only
pipe-controlled PID1 and nine 36-bit RAW PURE children. Each child
blocks on a real inherited pipe, is reclaimed through the native
scheduler, then receives one release token after the pressure check,
restores its writable suffix and exits. The original guest performs a
tenth native RUN and requires its fragmentation-driven refusal.
The host observes only original physical MM process/arena/extent
and storage data, using 71 SIMH physical write watchpoints from A to C.
It does not change KCORE, guest allocation code or scheduler behavior.

The linked KCORE DXRs match across both profiles, SHA256
`66296e13440e1f4db3bf263e181dc33e085f55dc4b0f07c09f147e6c1c88b506`.
The linked PURE MRES DXRs also match, SHA256
`834a99cc9d11668e761f679c7bf500be7aeff5d1db1216540edae9996ea27e49`.
Earlier mixed-p10fold-toolchain candidates were rejected; the matched
builds include the cross-object local-relocation identity fix.
The strict KINIT lacks the optional DISK SWAP MRES, as designed.

## Paired physical acceptance: PASS

| Actual MM state (48K = 49152 words) | GENERAL | STRICT |
| --- | ---: | ---: |
| A: free / largest free extent | 27339 / 27339 | 27339 / 27339 |
| B: free / largest free extent | 2692 / 1539 | 2692 / 1539 |
| C: free / largest free extent | 27339 / 27339 | 27339 / 27339 |
| Minimum free at source-linked committed MM event | 1255 | 1255 |
| Maximum nonfree physical words | 47897 | 47897 |
| Total original MM physical write events | 3122 | 3122 |
| MM count commits | 194 | 194 |
| Native in-place BRK changes | 5 | 5 |
| 128-word PURE drop / restore span changes | 9 / 9 | 9 / 9 |
| Other replayed MM descriptor writes | 2905 | 2905 |

The paired original `compare-v1674.py` passed. All 1660 physical MM
writes from A to B and all 1462 from B to C were replayed in order;
reconstruction agrees exactly at each original physical snapshot.
Both original `measure-v1673.py` owner/pin/transition/PROCINFO and
native SYS_MEMINFO acceptance oracles also PASS. Nine packed RAW owners
were physically present at B; their 128-word PURE prefixes were
reclaimed and then restored. Normal EXIT/WAIT fully recovers the
active extent table and all memory owners and pins. Inactive extent
slots can contain stale descriptor words, which are not valid owners
once `mm_extent_count` shrinks.

Both individual physical traces rejected five executed source-linked
witness changes (watch registration, original count commit, missing
MM write, PURE span corruption and BRK span corruption) plus a forged
summary. GENERAL and STRICT reports are byte-identical:

- v1674 high-water JSON SHA256:
  `088deea77704e6f5716045844013aea1a1fab7df3ecadf6e50880e46a7d00305`
- v1673 physical state JSON SHA256:
  `29e052c9372d0175aab02f6b39587e40edfca3d1184499d12f08d0f4a5f87a88`

## Limits / next priority

The 1255-word low point is the lowest valid *committed original-MM
state after a watched physical write* during this workload. Transient
intermediate sorted-table shifts are replayed but excluded from RAM
high-water; they are not actual committed owners. Host-side SIMH
watchpoint overhead may alter simulator clock/scheduler interleavings.
This is not a timing-neutral experiment, an instruction trace, a
universal scheduler worst-case bound or proof of successful 32K boot.

P1-E remains incomplete. Next: PI6 fault-pressure rollback and fault
injection across actual reclaim/restore, then ROOT=auto/DSK/DRM and
KCC FULL-profile integration regression. Bitmap compression mode3,
KA10 PURE ownership and final mainline integration are separate gates.
No claim of acceptance for those is made here.

Source of physical test contract:
`daimos-testkit/tests/validation/pure-runtime-highwater-v1674/README-v1674.md`.
