# makedepend DAIMOS port status — 2026-10-09

Upstream: X.Org makedepend 1.0.10, from
https://xorg.freedesktop.org/releases/individual/util/makedepend-1.0.10.tar.xz
The imported implementation and original COPYING are retained without
claiming that the original host-side Unix API is implemented by DAIMOS.

The portable host reference builds and scans includes. Native compilation is
currently BLOCKED by missing DAIMOS libc/PDP-6 interfaces, including
`fcntl.h`, `unistd.h`, `signal.h`, POSIX fd/stat interfaces, and a working
stdio implementation. The KCC self/include headers are compiler interfaces,
not evidence that all routines are implemented in the native libc.

Required before enabling a DAIMOS-native target:

1. Inventory and implement the required Unix file, directory, and stdio
   operations in the actual DAIMOS libc, rather than introducing host-like
   stubs or silently omitting preprocessing logic.
2. Rework upstream's path resolution and output redirection against
   DAIMOS's record/SIXBIT filesystem semantics.
3. Bound parser memory consumption for a PDP-6 process and measure peak RAM.
4. Build/link the program with KCC/DAS/DLINK and execute its native binary.
5. Compare conditional include graphs and the resulting dependencies against
   the validated host scan and test incremental rebuilds.

The included Makefile intentionally exposes only the tested host reference.
