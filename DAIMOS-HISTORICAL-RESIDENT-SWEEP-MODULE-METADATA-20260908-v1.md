# DAIMOS historical resident-memory sweep: module metadata de-duplication

Date: 2026-09-08

Base tree: `4ddc0ca729152c3a6d05290c8d97412f9bc6044a`

## Rationale

MM already owns the authoritative physical extent of every movable module.  The
runtime module table nevertheless retained a second two-word description per
owner: base/image span plus initialized/map span.  Only the current base and
initialized-word count are information that the module mover cannot cheaply
obtain from MM or derive.

## Change

Each runtime module descriptor is reduced from two words to one:

- LH: initialized words;
- RH: current physical base.

The relocation-map size is derived as `(init_words + 17) / 18`.  The module
extent size remains authoritative in MM and is passed to the cold module-move
routine by `mm_move_module()`.  Dynamic-binding retargeting still has O(1)
access to every module's current base.

During KINIT, the just-loaded module image size is kept in disposable KINIT
state solely for the bounds check used when registering dynamic bindings.  It
is not retained in KCORE.

Normal module calls, interrupt dispatch, filesystem dispatch, and other hot
runtime paths are unchanged.

## Measurement

Fresh same-toolchain PDP-6 links on the preceding expendable-init tree:

- previous `__kcore_low_init_end`: `014323`;
- optimized `__kcore_low_init_end`: `014320`;
- text reduction: 3 words;
- previous `__kcore_low_end`: `015103`;
- optimized `__kcore_low_end`: `015054`;
- total fixed reduction: 23 words;
- fixed-BSS reduction: exactly 20 words.

The table remains sized for all 20 owner slots; capacity/functionality is not
reduced.

## Gates

Green on the optimized tree:

- host `mm-v1`, including actual movable-module relocation and binding
  republishing;
- real five-process PDP-6 scheduler/preemption/RL-PL regression;
- protected 32K, 64K, 96K, and 256K boot/EXIT;
- 96K RAMFS threshold behavior.

No runtime call gains an indirection and no relocation map is discarded.
