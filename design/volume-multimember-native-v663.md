# Native multi-member VOLUME acceptance v663

The unchanged daimos-testkit v582 multi-member D6FS HDD suite was run
against DAIMOS HEAD following the v658-v662 DEVICE/VOLUME/BACKING
interface separation. Its first attempt failed before any actual
filesystem assertion: stale x86-64 `tcp-run-v1`/`pty-run-v1` host
executables had been inherited on an AArch64 host. This is a test
runner architecture mismatch, not a kernel failure.

Rebuilding the relay programs as AArch64 binaries and rerunning the
v582 suite produced `D6FS HDD multi-member runtime test PASS`. The
suite exercises equal-sized two- and four-member sets, file I/O,
large-file operations, and rejected mismatched/missing members.
The testkit v663 launcher now builds fresh, versioned host-native
relay programs on every run so this failure cannot recur from stale
shared build artifacts. This changes no DAIMOS production code.

The missing-runtime-VOLUME-mapper case is guarded by the v659 source
regression but is not yet demonstrated by native in-process fault
injection. That remains a separate mandatory acceptance item.
No KCORE code-size optimization was performed.
