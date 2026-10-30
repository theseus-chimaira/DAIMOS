# v761: Link-time compressed PURE executable backing (deferred)

Status: design idea only; no kernel, linker, or codec change authorized.

DAIMOS already supports DXR2 compressed executable payloads via D6LZ36
and a separate PURE executable promise. Disk swap already omits complete
0200-octal-word sectors of the immutable prefix; swap-in reloads that
prefix through executable backing, including compressed DXR2. Thus a
PURE executable compressed once at link time can serve as the source for
later text reconstruction without recompressing the text at swap-out.

Proposed optional build/link policy:

* Link the same executable in the existing supported compressed DXR2
  representation, measure actual encoded words and storage blocks, and
  compare against the raw representation after storage-block rounding.
* Record original decoded image length and PURE text boundary using the
  established DXR2/backing metadata, not an extra permanent process word.
* Decide whether compression is worthwhile using measured bytes/blocks
  saved and estimated PDP-6 decode cost, parameterized by storage type.
  DECtape latency can justify different thresholds than disk/drum.
* Mark the choice opt-in; a compressed DXR2 image is not equivalent to
  a runtime-COMPRESSIBLE process. The latter is a separate deferred policy.
* Do not invent a text-only executable format unless measured benefits
  justify changing the current whole-payload compressed DXR2 layout.

Runtime model: executing a process still requires its full, aligned,
decompressed resident image. On eviction, omit reconstructible PURE
sectors exactly as today and retain writable state in ordinary BACKSTORE;
on restore, reload PURE from the compressed executable and overlay the
saved writable suffix. Do not retain an additional compressed text copy
in scarce RAM by default. The backing vnode/mount must stay valid until
no nonresident process depends on it.

Measure representative real executables, link-time encode ratios,
initial load and repeated swap-in times, temporary decode RAM, exact
text/payload boundaries, and behavior on uncompressed, compressed,
PURE, and non-PURE combinations. Place any tests in daimos-testkit.

This idea does **not** address the v758/v759 deadlock: restoration still
needs the full resident extent. Keep the raw-swap pressure bug independent
of optional executable/text compression and in-RAM PACKED processes.
