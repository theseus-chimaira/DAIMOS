# v1365: grouped KINIT Stage-1 device boot acceptance

Date: 2026-11-08. Status: test-only grouped boot images and native
PDP-6 Stage-1 media paths. Production D6LZ36 remains in effect.

## Real KINIT input and isolated grouped image generation

The testkit v1365 builds the *actual* current linked full-profile
KINIT (`system/boot/pdp6` target `kinit.words`), including the
production initialized KCORE image installed into its reserved
linker slot by `image/mkimage.sh`. It does not create an artificial
small KINIT or replace the installed production boot artifacts.

The host test generator reads KINIT's two-word DAIMON/extent header,
checks the declared 18-bit decoded image size and entry, validates
the complete 36-bit payload, and compresses all KINIT words using
the independent staged grouped reference from the v1359 testkit.
The host decoder reconstructs every input word exactly. The separate
test-only boot stream uses the existing three-word boot framing:

    SIXBIT DAIMON
    decoded_words,,entry_offset
    grouped_compressed_words,,0
    grouped_payload...

The completed full KINIT has 32,670 decoded 36-bit words,
29,023 grouped compressed 36-bit words, an entry at octal 053524,
decoded image end 0127636, and staged input end 0220375.
Both ends fit below the configured full-profile 256K physical
limit `01000000`, including `0400` words of KINIT stack reserve.

The producer refuses malformed source headers, nonrepresentable
lengths, insufficient physical core or stack headroom, and an output
path equal to its input. It writes a temporary file only after
successful independent host roundtrip and publishes it atomically.
No `.d6lz.words`, production `kinit.words`, root filesystem image
or boot mode setting is changed by these tests.

## Native Stage-0 and five real Stage-1 media paths

The v1365 acceptance builds the v1364 guarded direct-linked grouped
Stage-1 sources for DSK, DRM, DTC, MTC and PTR. Each is assembled
and converted to a read-in paper tape at 000060. Actual SIMH
PDP-6 controller boot transfers use the grouped KINIT image:

| Stage-1 | Controller | Real test media | Result |
|---|---|---|---|
| DSK | DSK270 | Four-member DBOOT disks from `mkdsk` | PASS |
| DRM | Type-236 drum | Four-member DBOOT drums from `mkdsk` | PASS |
| DTC | Type-136 DECtape | `mkdt` virtual DECtape | PASS |
| MTC | Type-516 magnetic tape | `mktap` seven-track image | PASS |
| PTR | Type-760 paper tape | Two `words2pt` tapes and `TAPE2` switch | PASS |

These are actual Stage-0 read-in loader, Stage-1 controller, full
compressed-stream decode and KINIT entry-transfer tests, **not**
only extracted handoff snippets or isolated codec fixtures.
All five PDP-6 tests stop on the expected KINIT entry breakpoint
`053524` and compare the first reconstructed physical KINIT word
at 030000 against the linked source image.

## Failure behavior and strict limit of acceptance

Two full Type-136 negative boot media are also tested:

- A forged maximal decoded length must fail the v1364 memory
  geometry guard before any KINIT payload store.
- An invalid zero-token control word in the grouped compressed
  stream must fail the native decoder before any KINIT payload store.

Both native simulated boots halt instead of entering KINIT and
leave physical address 030000 unchanged. Host-format generation is
also refused for an impossible 32K profile or an excessively large
stack reserve without publishing a partial image.

This proves the **outer Stage-1 boot compression format** for the
five existing media paths and two DECtape error cases. It does not
prove that the KINIT runtime can operate with compressed grouped
executables or TSFS/D6FS files: KCORE, VFS, EXEC, PURE, swap and
filesystem consumers still expect production D6LZ36. We stop at
KINIT entry, not at INIT/LOGIN. Future work must check additional
decoded image locations, device errors and premature media EOF
on all devices, and full KCORE consumer/caller stack costs.

Production conversion requires a synchronized update to every host
producer, Stage-1 artifact, runtime consumer and image in a single
cutover with no permanent legacy fallback or dual codec residency.
The v1365 branch makes **no** such production change.
