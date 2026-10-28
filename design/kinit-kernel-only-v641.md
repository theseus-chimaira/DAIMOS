# Native 32K KINIT-only initialization v641

There is deliberately no `PROC_BOOT_USERS=0` production mode:
`kinit_late.c` rejects boot-user counts below one. To validate the
32K kernel without the shell or userland workload, testkit
`daimos-kinit-kernel-only-v641.sh` builds a standard GCC lowmem
tape-root boot with its normal process policy and derives the
`proc_boot_init` address from the final linked KINIT map.

The test inserts a SIMH breakpoint at that entry, so KINIT can
complete early hardware and filesystem initialization while process
initialization and all userland execution remain unentered. It also
checks that the linked image leaves the original octal `0600` stack
reserve within the 32K limit, and that DTC, DSK270 and DTFS have
reported successful initialization.

Native PDP-6 SIMH PASS: the standard 32K image prints `MEM 32 K`,
`DTC OK`, `DSK270 OK`, `DTFS LOADED` and stops at `proc_boot_init`
octal `061423`, without printing userland `INIT V1`.

This is a test-only stop before userspace, not a new kernel mode or
a change to normal boot behavior. It does not replace the separate
32K end-to-end userland boot test (v493 already PASS), 48K/96K/256K
profile acceptance, or exhaustive KINIT error-path stack tests.
