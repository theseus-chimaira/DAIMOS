# D6 codec v055 — native kernel ABI staging

The production-format D6 codec v053 source was imported unchanged from
`de85d60`. The staged `d6codec_kcore_abi_v055.s` preserves AC10..AC16
around the complete-buffer native encoder and decoder. Its entry points
are **not** linked into the live kernel, and it deliberately does not
replace the resumable 000060 decoder or the identity kpack implementation.

The testkit regression `tests/validation/d6codec-kcore-abi-v055` reuses
the v051 full native `darc.dxr` golden corpus and verifies bit-exact
output, exact decode, and callee-save preservation for both entry points.
Run with `DAIMOS_REPO="$PWD" ../daimos-testkit/tests/validation/d6codec-kcore-abi-v055/run-v055.sh`.

**Blocking format migration work:** The v053 decoder consumes a complete
buffer and cannot implement the old 000060 core's resumable NEED_INPUT
interface as a straight binary replacement. Its complete stream format
also differs from D6LZ36. Thus stage-1 bootloaders (all five devices),
KINIT compressed image production, VFS refill, DXR, PURE backing, TSFS
restart consumers and host producers must be migrated and tested as one
compatible image/artifact cutover. The v054 checklist records the complete
acceptance requirements. Until that succeeds, disk swap and the existing
bootable D6LZ36 path remain untouched; no production compression claim.

The wrapper currently has a seven-word saved-register frame in addition
to the native engine's temporary stack use. The v051 recorded six-word
canary stack measurement is not a worst-case nested KCORE-stack bound.
Full stack high-water and linked size must be remeasured at activation.
