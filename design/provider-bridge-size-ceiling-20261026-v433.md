# Proposal B v433: KCORE provider-bridge size ceiling

Status: source-based size audit; no production edit and no new RAM
reduction claimed.

The whole `fs_provider_reg_call` through `fs_provider_unlock` region of
`system/kernel/fs/fs_mres.s` contains **30 assembled instruction words**
(counting the eight entry/dispatch instructions and 22 instructions
in serialization, wait/retry and unlock). The four provider jump slots
are a separate four words and must remain or be replaced by bindings.
The single `fs_provider_ready` data word is also separate. The source
count is a bound for this *one* bridge, not a post-link delta.

The earlier unshared fast-path experiment grew linked KCORE by four
words and was reverted (v431). Safe transfer buffers remain live across
sleeping physical I/O and TSFS record consumers (v427-v429). Relocating
the bridge into an MRES would therefore need a pinned service that
cannot move/unload during active calls, a shared lease for all three
physical providers, and an entry binding in KCORE. It is **not** a
free 30-word reduction: its replacement binding and total physical
RAM cost need measuring.

Thus the greatest *possible* KCORE saving from eliminating all bridge
instructions is only 30 words before the unavoidable replacement ABI,
and likely smaller in practice. Do not pursue this as the principal
large-scale shrink: instead inventory larger resident FILE/VFS and
syscall-dispatch regions for opportunities removing complete layers.
Keep the accepted v432 KCORE **8990** unchanged. Strict 32K acceptance
remains deferred.
