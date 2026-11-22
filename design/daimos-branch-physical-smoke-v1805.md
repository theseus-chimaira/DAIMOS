# DAIMOS v1805 physical integration smoke

Date: 2026-11-22. Branch:
`kcore-raw-pure-integration-v1804`, based on main v1803
`495060f6` and RAW PURE development `5ef8b15a`.

## Real PDP-6 default-OFF guest

The v1804 integrated 256K DSK/disk-root image was built
successfully with native KCC, DAS and DLINK, then booted
under `$PDP10_PREFIX/bin/pdp6` in headless mode. The
guest emitted the following console markers:

- `MEM                                 256 K`
- `DSK270                                OK`
- `DRM236                                OK`
- `D6FS                              LOADED`
- `INIT V1`
- `LOGIN:`

The simulator remained alive until the controlled 45-second
timeout, returning 124; it did not HALT before that timeout.
The host's SIGTERM message after the deadline is expected,
not a guest crash. Evidence is stored in the v1805 guest
directory under `$TMPDIR`. The testkit v1805 reusable
smoke gate repeats these conditions on a fresh guest.

**Repeatability finding:** An initial attempt to repeat the
boot directly against the already-used image HALTed at
`074301` with `?RT` before INIT. This is an observed
second-boot failure, not a successful repeated boot.
The guest had previously written to the source disk set,
so that run was not a clean-image reproduction. After
regenerating the source disk set with `mkdsk`/`mkd6fs`
(D6FS CLEAN, sequence 1), the reusable testkit smoke
copied all DSK and DRM members into a per-run directory,
rewrote the simulator's attachment paths to those copies,
and PASSed through INIT/login with timeout 124.
The source media is now never directly attached by the
testkit smoke. The `?RT` on reused media remains an
unresolved D6FS recovery requirement, not a harmless
simulator artifact. A controlled same-profile two-boot
v1803 mainline comparison reproduced the identical
`?RT` failure on DIRTY sequence-2 media. See
`design/daimos-d6fs-reboot-baseline-v1806.md`.

This is **boot smoke only**, not a release acceptance:
the build has RAW PURE, bitmap mode3 and swapless-only
default-OFF; no pressure/reclaim, multiowner teardown
refusal, slot-0 transaction, 48K/64K PURE workload,
post-HALT disk check or K01-K04 live fault injection
was performed here.

## Memory and promotion boundary

Same-profile baseline/candidate size comparison is documented
in `design/daimos-branch-consolidation-v1804.md`.
Fixed KCORE decreases by 5 words; removable SWAP MRES grows
53 words, yielding 48 additional permanent-address words.
The v1666 terminal MRES service accounts for most of the
new SWAP source, but it is installed as a third exported
service, and removing it would break disk terminal ownership
semantics. No speculative deletion or ABI shortcut is
authorized to reclaim the 53 words.

The 32K strict permanent-address limit remains unmet on
both the pre-merge baseline and the integration candidate.
Neither repository main branch has been changed.
