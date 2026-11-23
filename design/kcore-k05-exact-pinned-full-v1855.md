# K05 v1855: exact one-pin FULL commit under PI exclusion

Date: 2026-11-23. Production fix: DAIMOS `1893c84b`.
Native KCC PDP-6 acceptance PASS; K05/K06-K09 remain OPEN.
RAW PURE and optional mode3 are OFF by default.

## The original unsafe commit

The v1839 FULL restore already delayed releasing the authoritative
PACKED RAW suffix until after `mm_unpin(target)` returned `MM_OK`.
However, an MM unpin decrements **one** pin and a success return does
not prove that the target has no *other* pins. A loader callback that
acquired a second original MM pin on the temporary complete PROCESS
APR therefore caused `mm_unpin(target)` to return success but leave
pin count 1. The transaction then released the sole PACKED RAW owner
and could publish the still-pinned PROCESS extent as RESIDENT.

An original KCC-compiled PDP-6 native test (`v1855`) reproduced this
before the correction: its new safety oracle failed with native
`__test_exit=000000002002` octal. A successful compile alone would
not qualify as proof; the original source actually executed and
reached that precise failure code.

## Production correction and memory cost

After the FULL loader and original mutable suffix overlay, the
already PI-excluded commit window now requires exactly TWO distinct
original MM owners for the selected process: a PACKED_PROCESS extent
at the full RAW suffix base and original RAW word count, and a
complete PROCESS extent at its independently allocated target base
and trusted APR word count. Each must have **exactly one** physical
pin. Missing, foreign, duplicate, changed-size or extra-pin records
fail closed with `MM_ERR_BUSY`: no speculative pin decrement, no
release of the original RAW suffix, no RESIDENT publication, no
READY_ONCE, and the existing TRANSITION quarantine remains held.

The original v1839 lost-source-free reply check and this new
precommit check now share `pram_exact_owner_set_v1855()` under PI
exclusion. Pass expected pin count 1 before commit, 0 when
reconciling a failed `mm_free(PACKED)` return. In the latter case the
exact sole PROCESS owner means source free succeeded; both owners
still live means target rollback is safe; anything else remains
quarantined. No new persistent registry, process bit or kernel data.

This preserves the existing one-scan source-free policy and reduces
opt-in MRES size versus separate scanner implementation. Native KCC
standalone instruction-shaped assembly listings grew from 612 to 649
lines (+37); the rejected duplicate-loop variant reached 720 (+108).
These lines are NOT linked instruction counts. On a matched source-
built 48K PDP-6 configuration the actual removable PURE MRES DXR
image grows from `004000` to `004027` octal words (**+23 decimal**),
with BSS zero. The original fixed KCORE DXR is BYTE-IDENTICAL to
the previous accepted v1845/v1847/v1848 KCORE (SHA256
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`).
The new PURE DXR SHA256 is
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.

## Source-linked native evidence

The testkit `tests/validation/pure-k05-pinned-commit-v1855/`
compiles current production DROP, PREFIX and FULL C for native PDP-6
with KCC and links the actual PURE MM rebase/prefix assembler,
production storage-mode assembly and original v1395/v1839 fixtures.
The MM allocator/pin/free methods at the test boundary are native
source fixtures; they must not be confused with a whole-machine
physical allocator/PI6 fault injection.

* Two persistent `MM_ERR_NOMEM` allocation refusals preserve the
  exact unpinned PACKED owner, backing, mutable suffix and PI balance.
  A third restore after lifting the refusal publishes the complete
  reconstructed target without a remaining pin.
* One independently owned pinned PROCESS extent survives both a
  healthy FULL commit and each subsequent refused overpin fault.
* A second original MM pin on the target OR source inside the loader
  leads to `MM_ERR_BUSY`, two untouched reservations, exactly
  preserved MM pins, original backing/mode and TRANSITION. An
  immediate unsafe repeat call is refused without mutation.
* Two independently compiled unsafe source variants execute and
  are rejected at original PDP-6 exit `000000002002`: omitting the
  precommit check, and accepting pin count greater than one.
* The original v1839 lost-source-free reply suite and its two
  separate executed negative mutants PASS unchanged. The historical
  v1654 unsafe late-unpin mutant's source anchor was updated to
  preserve v1855's new precommit guard, and all v1654 native tests
  (including the old unsafe ordering and permanent unpin failures)
  PASS. Test-only fix: daimos-testkit `23ea31c`.

## Source-built integrated 48K PDP-6 normal restoration

The source-matched current integrated PDP-6 48K low-memory build with
opt-in RAW PURE and selected TSFS root rebuilt successfully after
v1855, with one root-backed plain and four independent secondary
DTC3-backed compressed PURE children. Its exact original five-child
pressure/PIPE/data/EXIT/WAIT workload reports (all octal):

    aaaaax[B013060/043565]<010000+010000>[A013060/043565]{013060-013060}LwwwwwZ
    HALT instruction, PC: 000000

The physical runner returned zero: `PASS lifecycle: physical 48K
source-checked PIPE/WAIT/EXIT`. Its unchanged permanent KCORE DXR
SHA256 is the same as v1845-v1848. The newly linked opt-in PURE MRES
has 004027 image words, no BSS, and SHA256
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
The prior MRES size was 004000; the physical guest-reported budget
changed from 043536 to 043565 octal, the exact 000027-octal difference.
The completed original physical console trace has SHA256
`c090f8c2aa9475d876286f37c7cd6ab036a4f77b64353730f98a785c95335c8e`.
Evidence: `$TMPDIR/pure-brk-pressure-v1614.qnobNT/` and the
testkit v1845 current-source five-child builder, run with v1855.

This physical acceptance verifies normal integrated restore after the
production correction. It **does not** physically inject an unknown
extra MM pin into the running 48K OS: the exact two-pin failure and
bystander preservation are verified by native PDP-6 tests only.

The full `make test-pure-integration-native-v1855` matrix PASSes,
including v1677/v1678/v1679/v1680/v1681/v1684, v1804/v1807/v1808,
v1839 and v1855 source and unsafe-mutation checks. This is a native
test matrix, not a replacement for the physical pressure acceptance.

## Remaining physical and release gates (uncertainty)

The new native overpin case is an exact source-linked MM test,
not a hardware injection into a multi-process full system.
Physical pin corruption and bystander scheduling progress must be
tested separately. Other outstanding K05 cases include persistent
source/target unpin/free errors, allocator foreign ownership,
all PI6 interleavings, alternative roots, continuous original MM
high-water, 64K/KA10 and K06-K09 acceptance. No production PURE
enablement is authorized by this correction.
