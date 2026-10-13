# D6 inline runtime integration v070 (inactive)

The v067 inline fixed decoder is paired with a separately named v070
runtime adapter. This adapter retains the original D6LZ36 64-word VFS
input window and 72-word stack frame. No production symbols are replaced.

Staged consumer copies refer to the new symbols:

- `fs/tsfs_runtime_inline_v070.s`: direct-buffer TSFS extent decode
- `proc/exec_load_inline_v070.s`: executable VFS streaming decode
- `proc/proc_pure_backing_pdp6_inline_v070.s`: PURE backing VFS decode

The standalone PDP-6 link probe passes. **These are source-level integration
stages, not end-to-end TSFS/EXEC/PURE functional regressions.** The staged
consumers must only be linked together with matching inline-coded images.
Format 1.3 on-media flags and version checks must be added to the runtime
TSFS parser before accepting images from `mktsfs-v069`; the current runtime
parser still recognizes legacy 1.1. Production symbols and boot images
remain unchanged. Final 8K KCORE limit remains mandatory; code-size
optimization is deferred by request.
