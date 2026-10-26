# Proposal B v438 — common OPEN descriptor result

The FIFO success path in `file_open` previously copied the allocated
descriptor number from AC12 into AC1 and jumped to `file_open_return`.
The regular-file path already performs that same publication at
`file_open_success`. Reuse that existing path after FIFO descriptor
initialization; the FIFO error path still clears both descriptor words
and returns -1 separately.

Full linked PDP-6 GCC lowmem result from v437 to v438:
KCORE 8989 -> **8988**; KCORE+MRES 15536 -> 15535;
PERMANENT_LAST 15583 -> 15582. No extra stack or permanent RAM.

The v437 static OPEN lifetime guard passes. Attempting
`test-daimos-pipe-fifo` did not reach runtime assertions: the build
failed because `DSH image 036013 exceeds EXEC limit 036000` (octal).
This is an unrelated DSH image-size blocker and **not** a FIFO PASS.
Additional FIFO/filesystem integration testing is required for final
acceptance. This is a small
KCORE improvement, not the cross-layer OPEN redesign. Strict 32K
acceptance remains deferred.
