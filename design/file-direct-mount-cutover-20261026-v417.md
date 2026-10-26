# Proposal B v417: accepted direct-mount FILE cutover

Status: linked and focused PDP-6 tests PASS; accepted 110-word permanent
image reduction. Broad boot/multiuser acceptance remains deferred.

User approved process-local vnode-wide advisory FLOCK and FILE-limited
three-bit provider/local-kind identities. The resulting two-word FILE
descriptor preserves a full 36-bit independent offset and stores all six
mount-ID bits directly. Its metadata occupies only the six unused bits;
the per-process u-area mount-extension bitmap and family token are gone.

Production consumers changed together: `fs/file_runtime.s`, `fs/file.c`,
`fs/file.h`, `fs/vfs.s`, `ipc/pipe.s`, `proc/proc_run.s`, `proc/proc.h`,
and `proc/proc_pdp6.s`. RUN copies complete descriptors without a
bitmap; PIPE and TSFS are distinguished using their full vnode; FLOCK
updates all current-process same-vnode descriptors, and regular OPEN
inherits this state. VFS CREATE reads UMASK9 in the credential word,
not the obsolete bitmap field. No additional dynamic allocation.

Fresh GCC PDP-6 lowmem full-linked measurements:

                        baseline       v417      saved
    KCORE                   9109       8999        110
    KCORE+MRES             15657      15547        110
    PERMANENT_LAST         15704      15594        110

No per-process physical allocation reduction is claimed: u-area stays
the same allocated size. The savings above are full-linked permanent
image words. This preserves the deferred strict 32K acceptance policy.

Focused daimos-testkit results: native FILE target PASS, native FILE
path walk PASS, direct mount 2/26/63 and FLOCK/DUP/CLOSE PASS,
native PIPE-vs-TSFS references PASS, native VFS UMASK9 PASS, cutover
source guard PASS, host six-bit contract PASS. The broader PIPE word
FIFO simulator test PASSed with a separately committed v418 testkit
compiler-selection fix. This does not replace full filesystem, boot,
RUN and multi-process simulator acceptance.

Regression files are in daimos-testkit, including the new v417
directmount and umask test pair and updates to existing FILE tests.
