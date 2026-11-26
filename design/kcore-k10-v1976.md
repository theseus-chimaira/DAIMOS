# K10 v1976: executable eligibility and existing ABI acceptance

Parent ffbb003e. This implements the admission portion of the optional
mutable-RAM feature, not compression itself. RAW PURE and mode3 remain
default OFF. No new syscall, format word, u-area word or process slot word.

## Format and admission

DXR2 TX2 LH bit17 (0400000) is COMPRESSIBLE_MUTABLE. The remaining
0377777 bits describe the existing uncompressed text count; current
initialized-image bound is 0300000. Word1 RH BSS/COMPRESSED/PURE/RT
layout remains identical. Marked RT_REQUIRED is invalid; host DLINK
also refuses marked raw XIP/MRES construction. Native DLINK recognizes
--COMPRESSIBLE-MUTABLE. Old kernel rejects the marked text size; new
kernel accepts legacy files. This is backward compatibility of new
readers, not old-reader forward compatibility. Host DXR tools retain
legacy text-count handling for non-DAIMOS images above 0300000.
DAS dxrcheck/dxrconvert still operate on uncompressed executable input.

Selected administrator approval policy: root-owned regular file with
no group/other write permission, validated after existing reservation
and access checks; ordinary UID may RUN/EXEC it. No inference from
PURE or payload COMPRESSED. Non-PURE marked executable receives the
eligibility bit but does not gain a PURE backing prefix. Shared parsed
image descriptor retains marker in internal flag bit0, not BSS.

Credential bit31 is distinct from UID/GID/UMASK and original mount/provider
identity. SIDL RH bit5 is staging-only; bit1 remains rollback, bits0,2..4
remain backing identity. RUN clears inherited executable identity and
copies the child's new five bits; staged EXEC saves/replaces/restores all
five. Destructive EXEC attaches directly to stable u-area; detach clears
only after terminal success. Late KINIT now retains SIDL until copying
staged identity to new u-area, then publishes SRUN. Future FORK must inherit
eligibility only; no FORK implementation exists in the current ABI.

## Acceptance and installed host tools

Matched lowmem PDP6 KCC baseline ffbb003e versus v1976, same profile:
10840 -> 10869 initialized words; BSS269 unchanged; 11109 -> 11138
resident words. First candidate +32, direct packed-bit extraction saved
three. KA10 candidate 10883+270 (no matched KA10 delta claimed). No new
per-process words. This is a fixed admission cost even when mode3 OFF;
no net compression payback asserted. Full distribution builds still
emit pre-existing unused native-tool/command notes and GCC include-order
warnings; no new K10 build warning identified.

Testkit b67ca49, DAS d5b7696; evidence relative to TMPDIR:

- k10-v1976.FMwAWZ: matched baseline source archive/build, initial
  candidate link, host build and install-v1976/local.tar.gz.
  SHA256 3fa5610a6b31811716da92e0b980b937d6998a5d0e10eb80d72aeec1e904311d.
  Six binaries installed in PDP10_PREFIX/bin. Historical standalone
  daimos-tools tree is superseded by DAIMOS tools/host, not a second ABI.
- pure-mutable-host-v1976.zz6t5whe: installed tools exact marker,
  unchanged image/relocs, compressed payload, raw conversion, INITFS
  words and valid D6FS image; text overflow/RT rejected. D6FS negatives
  require the actual header diagnostic, not generic format failure.
- pure-mutable-admission-v1976.xpSTaR: 26 native loader/inspector/attach/
  detach cases and executed privilege mutant (exit3). Controlled VFS,
  VM allocation and terminal leaves, not full physical ownership test.
- pure-run-identity-v1924.mD17yM: 64 RUN credential combinations; missing
  provider and marker mutants fail (0100,077). No extra RUN instructions.
- pure-exec-handover-v1934.p6hiiU: nine staged EXEC transaction cases;
  old/new/release-hold and old-width marker mutants fail.
- k10-full-v1976.jeacNi: actual marked INIT 64K disk PDP6 and KA10, native
  lifecycle P then HALT. PDP6 linked size is the matched candidate above.
- daimos-exec-replace-v1-7978: initial FULL AUTO 15s observation timed
  out before INIT; unchanged longer replay simh-replay-v1976.out reaches
  <E then HALT. Not an unresolved boot regression. No code change.
- daimos-exec-replace-v1-16322: actual 64K disk non-root UID6/GID7
  marked replacement, 28 malformed EXEC/RUN rejects, identity/FD/cwd/
  session/TTY retained, <E then HALT.
- daimos-native-tools-v1-20822: real native DARC/DLINK and marker option;
  linked program reads its own DXR header and prints MUTABLE TX2 OK,
  NATIVE LINK OK. All six shell probes PASS.

Replay compact host/leaf matrix:

    DAIMOS_REPO=$HOME/git/DAIMOS make test-pure-mutable-admission-v1976

K10 is accepted within the existing RUN/EXEC ABI. K11 reverse traversal,
K12 exclusive scratch ownership, K13 atomic DROP, K14 FULL restore,
K15 lifecycle and K16 failure/recovery/net RAM remain separate. No
marked system programs are shipped by default, mode3 is not published.
