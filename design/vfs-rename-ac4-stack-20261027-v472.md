# Proposal B v472: omit redundant AC4 save in VFS RENAME

The resident `vfs_readonly` helper only uses AC1, AC2 and AC3.
Previously `vfs_rename` pushed AC1..AC4 before two readonly checks;
AC4 already survives both checks unchanged. Remove its PUSH/POP,
keeping the original fourth provider request argument in AC4.
Adjust the common failure unwind from four to three stack words.
The second parent vnode is still at stack offset -1.

Native PDP-6 VFS RENAME v455 and integrated mapped provider-gateway
v466 regressions PASS. Exhaustive provider/mount namespace guard
v450 PASS. Testkit v472 checks the saving assumption against the
`vfs_readonly` assembly and the three-word frame. No ABI or filesystem
semantics change, no added static memory.

Full GCC PDP-6 lowmem linked measurements, v471 -> v472:
KCORE 8962 -> **8960**; KCORE+MRES 15509 -> **15507**;
PERMANENT_LAST 15556 -> **15554**. Savings two permanent words.
Strict 32K acceptance remains deferred.
