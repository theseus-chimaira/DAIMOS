# v1363: staged direct-linked grouped D6CODEC Stage-1 cutover design

Date: 2026-11-08. Status: test-only Stage-1 prototypes and native
handoff tests, **not** a production image or Stage-0 device boot.

## Corrected architecture from full source audit

All five current PDP-6 Stage-1 implementations (DSK, DRM, DTC, MTC,
PTR) read the **complete** compressed KINIT payload into a disjoint
buffer starting immediately after the uncompressed KINIT window at
octal `030000`. The existing D6LZ36 core is resumable because the
same installed low-core source is reused by *runtime VFS*, not because
Stage-1 uses NEED_INPUT during decode. Stage-1 invokes the old core
once after filling the entire compressed buffer.

Consequently the Stage-1 grouped-codec conversion can call the
already-tested complete-buffer `d6codec_v053_decode` directly.
It does **not** need the separate 230-word native streaming decoder
or its 26-word continuation state. The latter remains relevant to
VFS and other bounded-source kernel consumers. This corrects the
v1360/v1361 assumption that Stage-1 itself requires resumability.

Stage-0 loads each Stage-1 at octal `000060`. Test-only variants
embed `d6codec_v053_decode` in normal Stage-1 text and `PUSHJ` to
its linked address. No BLT decoder copy to `000060` occurs, avoiding
the original 40-word installed slot and any self-overwrite hazard.
The assembler/read-in converter performs the ordinary Stage-1
relocation. This is distinct from copying an unrelocated v1361
native object over occupied low-core addresses.

## ABI and KINIT handoff

`d6codec_v053_decode` accepts AC1 compressed source, AC2 exact
compressed length, AC3 destination and AC4 exact output length;
returns zero on exact success or -1 on failure. All five staged
variants preserve their original media-reading and raw-image paths.

- DSK/DRM read their existing `kinit_stack_base` and `compressed_words`
  variables, then preserve the existing entry/member count handoff.
- DTC/MTC retain their header compressed length in AC4 through
  reading the whole image; transfer it to AC2 before placing the
  decoded length in AC4. They save/restore AC5 (entry address)
  and AC13 (decoded length) on the existing Stage-1 stack.
  No additional persistent BSS is required.
- PTR saves/restores AC16 (entry address) and derives its input
  cursor and lengths from `image_words` and `compressed_words`.

On decoder failure, partially reconstructed output remains invalid;
the existing loader's error path is entered without jumping to KINIT.

## Accepted sizes and placement

All measurements use freshly assembled original and staged Stage-1
sources, DAS `-F -K`, and Stage-0 read-in conversion at base `000060`.
The staged code/BSS fits below the loader's `020000` stack origin;
decoded KINIT still starts at `030000`.

| Stage-1 | Old text | New text | Difference | New BSS |
|---|---:|---:|---:|---:|
| DSK | 350 | 432 | +82 | 879 |
| DRM | 342 | 424 | +82 | 879 |
| DTC | 140 | 229 | +89 | 0 |
| MTC | 106 | 195 | +89 | 0 |
| PTR | 139 | 224 | +85 | 3 |

The existing 40-word low-core decoder is no longer copied in these
test alternatives. Additional Stage-1 code is transient boot code,
**not** permanent KCORE residency. The new decoder has no new static
data/BSS; the DSK/DRM/PTR BSS shown is preexisting loader storage.

## Test-only acceptance and remaining gates

`daimos-testkit/tests/validation/d6codec-stage1-inline-v1363` emits
all five alternative sources without touching production, builds
relocated read-in images, validates linked symbol addresses and
low-core/code/BSS geometry, and executes ten native PDP-6 handoffs
at load base `000060`. For each loader one case decodes a valid
grouped literal into `030000` and transfers to the configured KINIT
entry with correct stack; a second case rejects a malformed control
without corrupting the output guard or entering KINIT.

These in-memory handoff tests do **not** exercise actual disk, drum,
DECtape, magnetic tape or paper-tape controllers. Important gates:
validate full KINIT staging/end-address geometry **before media
writes**, rebuild KINIT and every compressed artifact atomically,
test real Stage-0/device boots including truncation and rollback,
and separately integrate the runtime VFS/EXEC/PURE/TSFS decoder
without a permanent D6LZ36 fallback.
