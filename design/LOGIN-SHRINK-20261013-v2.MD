# LOGIN resident shrink v2

All counts are octal PDP-6 words. This is a measured intermediate
step toward the one-APR-block (002000-word total) goal, not its
completion.

| Field | v1 | v2 |
| --- | ---: | ---: |
| Image | 006133 | 004375 |
| BSS | 000255 | 000000 |
| Origin | 000020 | 000020 |
| Reserved stack | 001000 | 001000 |
| Required | 007430 | 005415 |
| Rounded APR | 010000 | 006000 |

The LOGIN-only link no longer includes libc/u.c's general-purpose
S6REC output sink, unrelated syscall veneers or the general APPEND
logging buffer. Minimal u_console.c output is sufficient for this
interactive program. The existing LOGSTORE MRES reconstructs full
persistent records when LOGIN submits four-word EVENT requests.
In addition to deleting unnecessary code, this eliminates the
128-word static logging record. The persistent on-disk schema is
unchanged; other programs may continue to use general APPEND.

Remaining high-cost operations include parsing SIXBIT S6REC PASSWD
records, validating names/passwords, packing five environment records
and constructing the SYS_EXEC_V1 block. A one-block executable with
00400 reserved stack has only 01360 octal words for image plus BSS.
Avoid migrating account parsing wholesale into KCORE: that simply
makes login-specific code permanently resident. Existing shared
kernel interfaces or removable MRES modules may be candidates only
with measured total system memory benefit and precise authorization
and concurrency semantics.

No behavior change was made to credentials, account-file formats,
terminal setup or shell environment. Runtime boot and persistent
LOGSTORE were validated on a new PDP-6 disk image. High-water stack
and low-memory 32K acceptance remain unverified.
