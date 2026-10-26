# Proposal B v403: sparse lock-family cost and lifetime

Status: design-only; no production descriptor change.

## Source-grounded behavior

`file_new_fd` assigns the opening FD as four-bit family token. `file_dup`
and `file_dup2` copy this token and a separate value of the 36-bit offset.
`proc_run_map_loop` copies the descriptor and offset into mapped child slots.
`file_lock` compares canonical vnode (including mount extension), then
family token; equal family bypasses conflicts and a lock update changes
all matching same-family descriptor states in the current FILE table.
Distinct OPENs of the same vnode are different families, even before
either acquires a lock. `file_close` discards the selected descriptor only
after successful closure; DUP2 first closes its destination. The target
test `tests/target/daimos-file-pdp6-v1/test.c` covers dup-family shared
lock update, independent-open conflict, exclusive conflict, unlock and
close, but does not exhaust all sparse-lifetime corner cases.

## Why an allocate-on-FLOCK table is not free

An initially unlocked family must survive OPEN, DUP, descriptor remapping
and CLOSE to identify all descendants when a subsequent FLOCK arrives.
Storing only active locks loses this equivalence unless the remaining
descriptor family graph is recorded or reconstructed from another stable
identity. Vnode or identical offset cannot serve as the identity: two
OPENs may name the same vnode with the same offset but distinct families.

There can be up to 16 simultaneously open families and arbitrarily chosen
equivalence classes among 16 FD slots; the current four-bit owner field
provides a fixed 64-bit family-label capacity. A proposal that eliminates
it must account for a replacement equivalence representation at all times,
not just the number of active locks. A sparse global table also needs
process ownership, lifetime, allocation and reclamation rules; a fixed
table becomes permanent RAM, while MM allocation has peak RAM and failure
costs. Neither is established as smaller by this audit.

## Correctness hazards to test before changing behavior

- DUP after locking, DUP2 replacing a member, and CLOSE of an originating
  FD while duplicates remain; opening an FD number again must not
  accidentally inherit the old live family's identity.
- Multiple independent opens of one vnode, including identical offsets,
  different mount IDs with identical provider/local indices, and simultaneous
  shared versus exclusive lock requests.
- RUN remapping to different child FD numbers, child/parent lock scope,
  process exit/forced teardown and unmount lock clearing.
- Failed CLOSE and failed RUN mapping must not partially transfer or erase
  ownership; locking cannot retain unbounded dead family objects.

## RAM comparison

Baseline descriptor metadata includes four owner bits per FD (64 bits)
inside the packed vnode word and no separate owner allocation. The current
mount-extension map uses one u-area word per process. A canonical-vnode
layout needs a separate representation of mode, type/APPEND and lock state
as well as family identity. A sparse family allocator removes no existing
process words unless a replacement encoding for ALL other flags is also
proved. It additionally pays dynamic words, allocation metadata and
release costs whenever families exist. There is therefore no presently
demonstrated net physical RAM saving from the sparse alternative.

The process u-area is 0410 octal (264 decimal) words; reserving a further
three metadata words in it would raise useful committed process storage
by three words if the u-area is actually resized and allocated at that
granularity. Check allocation rounding before comparing physical RAM.

## Next decision

Do not implement a sparse lock table yet. First consider whether a
well-defined new *per-process* FLOCK contract can remove family identity
entirely while preserving necessary exclusion and DUP behavior; this is
explicitly a semantic redesign needing approval, not a compatible
compression. In parallel, enumerate reachable flag combinations and
design a bit-exact packed per-FD mode/lock map. Reject designs requiring
slow provider calls on ordinary READ/WRITE. Keep existing packed format
as control and compare fully linked PDP-6/PDP-10 size, per-process and
peak MM RAM, plus hot-path execution time before acceptance.

Reference only, not remeasured: v388 KCORE 9109; KCORE+MRES 15657;
PERMANENT_LAST 15704. Strict 32K and JRST folding remain deferred.
