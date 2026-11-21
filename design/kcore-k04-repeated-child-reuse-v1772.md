# K04 v1772: live repeated child completion and slot reuse

Date: 2026-11-21. Status: normal-case physical PDP-6 smoke PASS;
MM/FILE refusal and multi-owner acceptance still OPEN.

## Method and observations

Used the clean, fresh v1770 256K PDP-6 DSK image on the installed
Open SIMH PDP-6. The first emulator attempt terminated with host
signal 11 shortly after TTY LOADED. The disk was still CLEAN
(D6FS superblock 742, sequence 5). The second invocation of the
same image booted normally through D6FS LOADED, INIT V1 and LOGIN.
The first attempt's host crash is not attributed to the guest
kernel; it is an unresolved intermittent emulator/host failure.

Logged in as ROOT (DSH V1). Issued five consecutive `TRUE`
commands. Every invocation returned to the shell prompt without
error. Issued `PS`; it showed PID 5 as the running PS process,
with INIT PID 1, USER PID 2 and LOGIN PIDs 3/4. All displayed
TD, RETRY, PVM, DETAIL and ERR fields were zero. This shows that
the ordinary shell-child slot was available for reuse after the
five TRUE completions. It does not demonstrate reuse while an
MM or FILE provider refuses release.

Issued guest `HALT`; SIMH exited normally at octal PC 022464.
Read-only `d6fsck -n 4 -d <diskdir>` then reported superblock
742, sequence 7, CLEAN, 179196 blocks and 360 FCBs.

## Acceptance boundary

This is a manual production-image smoke observation, not an
injected test fixture. Do not modify the external testkit while
its separate sweep is in progress. Permanent/temporary MM and
FILE refusals, simultaneous owners, deferred HUP, clock wrap,
parent death during EXIT, WAIT semantics under refusal and safe
PID reuse after the *last exact resource release* remain OPEN.
The source-backed acceptance plan is
`design/kcore-k04-multiowner-acceptance-plan-v1771.md`.
