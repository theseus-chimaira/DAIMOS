# v439: DSH EXEC size gate and FIFO integration recovery

The v438 FIFO test blocker reproduced the earlier v418 fixture
configuration defect: `daimos-pipe-fifo-v2.sh` compiled its replacement
INIT with GCC but omitted `COMPILER=gcc` for the full boot image.
Consequently the submake selected KCC for DSH and failed with
`DSH image 036013 exceeds EXEC limit 036000` (octal).

Testkit v439 explicitly passes `BOOT_PROFILE=full COMPILER=gcc` to the
test image submake. It does not raise the DSH gate or alter production
DSH source. A fresh run of `daimos-pipe-fifo-v2.sh` now **PASSES**
its PDP-6 end-to-end checks: MKFIFO, OPEN rendezvous, shared runtime,
rename, unlink reuse, mount-busy and D6FS persistence.

This fixes the build/test configuration, *not* KCC DSH size. The
historical full-profile GCC DSH image 031313 fits the 036000 gate;
the KCC DSH 036013 still does not. Further code-generation or source
optimization is required to make KCC-generated DSH fit independently.
Do not claim a DSH code-size reduction or a KCORE change from v439.

KCORE remains at the v438 linked baseline of 8988 words. Strict 32K
acceptance is deferred.
