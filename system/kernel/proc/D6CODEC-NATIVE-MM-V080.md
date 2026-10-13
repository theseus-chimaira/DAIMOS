# Native PDP-6 packed-process MM transaction v080

The daimos-testkit native v080 fixture links the actual PDP-6 GCC output
for `proc_packed_evict_v077`, `proc_packed_restore_v46`, the v079 public
API, packed registry/layout/owner support and the v078 assembly codec.
Only the MM allocator, extent descriptors, process records, and PURE
reloader are replaced by deterministic PDP-6 test fixtures. The fixed
v067 decoder is installed at 000060 before execution.

Four native cases PASS: dense and sparse packed descriptor layout, each
with a zero-dominated or mixed literal/zero writable suffix. Each case
checks successful compressed eviction, unpinned MM packed ownership,
restoration and PURE prefix contents, release of packed ownership,
registry retirement, and return to resident storage mode.

An earlier harness hang resulted from a cross-object symbol-difference
in the fixed decoder BLT end operand, assembled as 000057 rather than the
87-decimal-word endpoint 000206. The test now uses the verified fixed-image
word count explicitly. The first restored-image failure was due to a
512-word-aligned destination where process MM requires 02000 octal word
alignment; the fixture now selects an aligned region from a larger array.
The repeated-case fixture also resets registry metadata before each case.

All earlier packed host-MM v45/v46/v076/v077/v079 and native codec v078/
v079 regressions continue to pass. **This is not a full DAIMOS scheduler
boot test**: real MM arenas, scheduler exclusion, lifecycle policy,
production swapping and TSFS image migration still need acceptance.
The user deferred resident-size optimization for this stage.
