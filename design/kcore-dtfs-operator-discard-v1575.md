# KCORE v1575: trusted-operator DTVOL2 RW and poisoned-owner discard

Date: 2026-11-14. P0-B CLOSED under the explicitly accepted
**trusted-operator media contract**, not under the earlier v1571
hardware-enforced interlock requirement. VERSION 0.5, 32K HOLD.

## Operating assumptions and release boundary

The operator must NOT remove or exchange a physical DECtape reel while
any writable VFS mount still owns the reel. A poisoned mount remains
owned until a privileged, explicit destructive discard has finished.
The operator must quarantine a damaged or abandoned reel; a V1 legacy
volume with no durable WRITE_INTENT has no on-media evidence of an
interrupted write. The documented DTVOL2 durability contract instead
uses a complete 72-bit stamped RBN0 and salted DIRTY/CLEAN seal.
This site policy is intentionally weaker than a hardware reel lock:
v1571 proved the unchanged Type-551 cannot detect silent physical
exchange. The trusted-operator policy does not excuse raw I/O errors,
stale cache/owner use, incomplete DMA, interrupted writes or failure
to release MM pins. Those retain their production checks.

## Production changes

The pre-existing v1572 source enables production writable V2/CLEAN
admission, removes only the duplicate v1555 unconditional read-only
veto and retains the exclusive v1561 VFS unit/policy/owner gate.
Actual physical DIRTY intent is committed before native directory
or DATA writes; each already-armed write retains its physical v1554
RBN0 verification and pinned scratch until separately optimized.
Final unmount writes and verifies CLEAN while holding its exclusive
VFS barrier. Any raw partial write retains WRITE_UNCERTAIN, refuses
normal FILE/cwd/UNMOUNT and fails a later DTVOL2/DIRTY MOUNT.

The v1575 administrative policy changes the private operation 026
from `VERIFY_REPLACE` to `DISCARD_DRAIN`. It still executes under
the *same serialized DTFS provider lease*, so in-flight provider
I/O cannot overlap the final VFS owner and poison rechecks. The
private service invalidates `dtfs_cache_mount` and clean BCACHE
tags. It does not issue ANY physical RBN0 or directory read,
compare the newly attached volume's identity, alter WRITE_UNCERTAIN,
repair or reformat the old reel. VFS retains the initial and final
exact root/noncritical/poison, FILE/cwd/PIPE/swap and MM scratch
checks, plus `file_unlock_mount` and atomic slot/identity retirement.

The privileged legacy syscall spelling `REPLACE_SLOT` retains its
ABI for compatibility, but means **DISCARD_POISONED_SLOT** under
operator trust. After the call, a separate normal MOUNT performs
its own fresh physical directory and full V2 RBN0 authentication.
Mounting the original DIRTY reel still fails on both RO and RW.
Legacy V1 has no equivalent durable DIRTY proof; its exclusion
depends on operator quarantine/reformat discipline.

## Actual physical and source-linked acceptance

Production-shaped full 256K KCC/PDP-6 Type-551 guest:

1. Mounted V2/CLEAN RW, physically wrote first WRITE_INTENT and DATA,
   performed further writes, physically CLEANed/unmounted, and a
   new production cold boot read the same reel (v1572).
2. A genuine Type-551 RBN0145 partial DATA write changed exactly one
   requested 36-bit word, retained the original complete DIRTY RBN0;
   the poisoned mount refused use and the next real cold boot refused
   both RW/RO with the original reel unchanged (v1572).
3. A real armed RW/DIRTY read error/return verified the original
   full physical identity and then CLEANed without other block writes.
4. v1575 real PID1 held FILE/cwd and observed a one-word DATA tear.
   Clean/admin actions and implicit CLOSE were refused; only explicit
   root DROP_FD and cwd release removed owners. The privileged discard
   retired the poisoned mount **while the same DIRTY reel was still
   present**. Fresh RO and RW mounts of that reel both refused. Only
   after the operator attached a separately formatted V2/CLEAN reel
   did independent normal RO mount, STAT, CREATE refusal and UNMOUNT
   succeed. The entire old DIRTY and new CLEAN media SHA-256 values
   remained individually unchanged. Another full production cold boot
   of the original DIRTY image refused both mount modes.

Native source-linked owner suite: real VFS/FILE all-process u-area
scans and an injected remote FILE opener during provider-lease wait
PASS; four executed unsafe mutations fail. The real new DTFS drain
leaf clears cache state and calls BCACHE invalidation exactly once;
two executed skip-cache and skip-BCACHE mutations fail. The separate
v1574 Type-136 reverse-write overrun is fixed and physically tested.

## Size and remaining work

Compared with the same v1572 source profile before v1575, the
optional DTFS MRES DXR shrinks from 23,760 to 23,048 bytes:
712 bytes = 89 packed 36-bit words. The native `dtfs_lookup`
address moves back 0126 octal (=86) TEXT words. No new fixed
KCORE, mount slab, MM scratch, ABI or on-media format; the fixed
`__kcore_low_end` remains 024364 octal. The v1574 independent
Type-136 driver fix costs one optional tape MRES instruction.

P0-B is CLOSED for the **documented trusted-operator DTVOL2 contract**.
It does not claim hardware-controlled reel continuity, durable
blacklisting of V1 legacy tapes or automatic repair of DIRTY media.
P1-C combined swap/MEMFS persistent snapshot/LOGSTORE physical fault
isolation remains OPEN; P1-D/P1-E remain separate. No 32K shrink
or speculative filesystem redesign was introduced.
