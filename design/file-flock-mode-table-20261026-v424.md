# Proposal B v424: branch-free FLOCK state replacement

Status: accepted; linked-image and native PDP-6 FILE tests PASS.

The previous `file_lock_update_loop` special-cased UNLOCK (mode=0) by
branching to insert the REGULAR state bit. The new `file_lock_modes`
table contains the desired resulting state directly: SHARED=020,
EXCLUSIVE=030, UNLOCK=010 in the descriptor's high half. Clearing the
old 030 bits, then OR-ing the selected table entry, now handles all
three operations identically. No field semantics, ABI or allocation
changes; this removes three linked resident instructions/words.

Full GCC PDP-6 lowmem image:

                     v423      v424      saved
KCORE                8998      8995         3
KCORE+MRES          15546     15543         3
PERMANENT_LAST     15593     15590         3

Native daimos-testkit FILE target and directmount suite PASS, including
lock transitions, mount 2/26/63 isolation, DUP/DUP2, close/unmount,
PIPE/TSFS identity, and UMASK9. Net saving since pre-v417 baseline:
114 permanent words. Process u-area allocation unchanged. Strict 32K
acceptance and unrelated JRST folding remain deferred.
