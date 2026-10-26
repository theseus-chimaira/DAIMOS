# Proposal B v450: one namespace comparison for VFS RENAME

RENAME previously decoded and compared the six-bit provider and the
six-bit mount-id independently. The combined upper twelve vnode bits
(provider 35..30 and mount 29..24) form the namespace identity; local
kind and index must not participate. A single 36-bit mask
`0777700000000` is applied to both vnodes before comparison.

An initial experimental mask accidentally omitted the upper three bits;
the exhaustive testkit namespace check caught this during development.
The corrected mask was rebuilt and independently remeasured.

Full linked GCC PDP-6 lowmem: KCORE **8966 -> 8963** (three words
saved), KCORE+MRES 15513 -> 15510, PERMANENT_LAST 15560 -> 15557.
No permanent data, stack-frame or public ABI change.

Testkit exhaustive 4096 provider/mount identities PASS, including
independence of local vnode kind/index; FILE/PIPE direct-mount, UMASK
and native PDP-6 provider-lock tests PASS. A complete RENAME syscall
integration test remains desirable. Strict 32K acceptance deferred.
