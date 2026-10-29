# KINIT MM allocation-failure watermark v650

The native GCC 96K full-profile KINIT with
`KINIT_STACK_WATERMARK=1` successfully reaches the genuine idle-stack
`mm_alloc` failure path. Testkit
`daimos-kinit-mm-alloc-failure-v650.sh` finds the unique idle-stack
allocation call sequence and KINIT entry from the linked map/image.
After Stage1 loads KINIT, SIMH breaks at `kinit_enter` and substitutes
an impossible `0777777`-word allocation request for the normal
`0100`-word kernel idle stack. No production source, disk, or persistent
boot artifact is modified. The allocator fails and KINIT HALTs before
root mounting or userland startup.

Measured fatal-path stack high-water is `0172` octal (122 decimal
words), below the unchanged `0600`-octal (384-word) stack reserve.
Both preflight and postflight D6FS checks report CLEAN. Native
regression PASS. Other independent I/O and missing-INIT fatal paths
remain to be measured.
