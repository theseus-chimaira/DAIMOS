# KCORE GCC object-size audit v10

Compiler: pdp10-dec-none-gcc -Os -march=pdp6 -mcpu=pdp6.
Profile: lowmem, clean independent GCC build directory.
Linked KCORE low end: 023361 octal; acceptance requires <020000 octal.

Largest selected object sections (decimal words, text+data+BSS, BEFORE
link-time folding and layout; therefore NOT additive as resident savings):

| Object | Total | Text | Data | BSS |
| --- | ---: | ---: | ---: | ---: |
| proc_pdp6 | 2162 | 2156 | 3 | 3 |
| file_runtime | 1062 | 1062 | 0 | 0 |
| monitorfs_runtime | 1031 | 992 | 39 | 0 |
| vfs | 873 | 713 | 24 | 136 |
| syscall_dispatch_pdp6 | 796 | 796 | 0 | 0 |
| mm | 590 | 520 | 0 | 70 |
| proc_swap | 524 | 522 | 0 | 2 |
| exec | 453 | 453 | 0 | 0 |

Priorities: (1) trim process scheduler/control and syscall hot/cold paths;
(2) investigate nonessential monitorfs/file runtime responsibilities;
(3) replace disk-backed swap with in-core process packing after atomicity
tests; (4) examine resident metadata before optimizing small PCLK (24 words).
Object totals are diagnostic only. Verify every proposed change using the
full folded GCC link map and runtime tests, not object totals alone.

## Repeatable measurement (v12)

Run `sh tools/host/kcore-size-audit-v1.sh BUILD_DIRECTORY` against a clean
GCC lowmem link. The diagnostic lists selected KCORE fixed objects, not the
entire image, and reports the actual map endpoint separately. In the current
link the endpoint is 023360 octal. MonitorFS remains 1030 decimal pre-fold
words, process control 2162. The disk and drum call-through objects together
account for just 37 pre-fold words; their dependency removal must also address
filesystem dispatch and legacy swap before a minimal build can omit them.
