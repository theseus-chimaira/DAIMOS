# K02 physical post-MM-free EXEC backing refusal v1721

Date: 2026-11-20. Status: accepted PDP-6 guest evidence, K02 still OPEN.

## Goal and exact failure boundary

The v1720 low-memory EXEC fix differentiates an MM refusal before the
PROCESS extent is freed from a refusal after `vm_space_destroy` has
published VM_NONE. The latter is irreversible and must enter the
private fatal-FILE/EXIT path, not restore the old user-map hold.

The source-linked native v1720 test had already validated both status
cases. This additional test uses a full booted 256K PDP-6 and the real
MM allocator/VM destructor. Only the selected backing-detach boundary
in a disposable copy of `proc_swap_detach` returns injected failures.
The production `vm_space_destroy`, `mm_free`, FILE close, timer, slot-0
finalizer and reaper are not substituted by test callbacks.

## Physical guest

New daimos-testkit files:

- `tests/validation/daimos-k02-postfree-detach-v1721.sh`
- `tests/validation/daimos-k02-postfree-detach-v1721.py`

The versioned generator checks exact anchors in the existing v890
full-system guest script. It creates a temporary guest variant and
adds test-only C instrumentation to the copied kernel's `proc_swap_detach`.
The instrumentation requires that the selected process's VM descriptor
equals the *full* VM_NONE word, and checks the actual `mm_extents[]`
table for any remaining MM_TYPE_PROCESS entry owned by that slot. It
records the backing word at the first invocation and checks it unchanged
on the next two. Two calls refuse detach and leave ownership intact.
The third executes the original implementation. Test-only counters also
distinguish process-context from slot-zero retry calls.

The original v890 assertions for FILE-close retry, preserved descriptor,
HUP/EXIT and WAIT remain active. Unlike the v890 success path, this
test deliberately fails old-VM destruction **after** MM free but before
the `exec_lowmem_old_gone` continuation. Consequently v890's old-VM
success, load-failure and final-destroy-success counters must all be
zero; this is checked explicitly, not ignored.

## Observed 256K PDP-6 guest

| Observation | Result |
| --- | ---: |
| Backing detach attempts | 3 |
| Injected backing refusals | 2 |
| Calls from original EXEC owner | 1 |
| Calls from slot-zero finalizer/reaper | 2 |
| Immutable original backing record | `037` octal |
| Remaining PROCESS MM owner on refusal | none |
| Test instrumentation errors | 0 |
| FILE close passes / VFS refusals | 3 / 2 |
| Fatal FILE retry sleeps | 2 |
| Final EXIT / parent WAIT | PASS |
| Unintended old-image resumption | none observed |

The final standalone v1721 script returns zero after building and running
the 256K PDP-6 guest and checking its sample counters. The guest reaches
its expected `HALT` after final WAIT. A separate verifier replay of the
earlier instrumented physical guest also passed the same assertions.
The first instrumented build exposed a test-only symbol-length problem:
KCC truncated two overlong diagnostic symbols in the link map. The test
uses shorter versioned names now. No production compiler or assembler
change was necessary.

For the final standalone accepted guest, the isolated KCORE DXR SHA-256 was
`dd1f57246b6c60d0b162a47c8ea286c1aee92df3173204f4bb8c97935dd92cfc`;
the accompanying SIMH log SHA-256 was
`2bccc574e5e3336d15127f9e8c756ec07de0e9c5056cbfec359fcf1beca69262`.
These identify the test-instrumented guest image, **not** an altered
production build.

## Additional K02 cleanup regressions

Legacy native tests v865/v867/v923 had stale FILE extraction boundaries
that pulled in unrelated DTFS administrative code, plus uncovered cold
foreign-event dependencies. Their test-only fixture extraction/stubs
have been corrected; each passes against production KCORE source.
The separate v870 PIPE final-MM-close retry and v892 unpublished RUN
MM rollback/reaper tests also pass. These changes affect testkit only.

## Remaining K02 and K04 work

This verifies an injected backing-detach failure **after a genuine
physical PROCESS MM free**. It does not prove that an actual disk-swap
or filesystem provider will independently return the same refusal,
nor every permanent-rejection or PI6 interleaving. EXIT, unpublished
RUN, FD, PIPE/FIFO and provider-level cleanup still require combined
failure-pressure acceptance. K02 is not closed, and K03 automatic OOM
termination remains prohibited. No completed-feature changes are made
to `pdp10-doc` until K02 closes.
