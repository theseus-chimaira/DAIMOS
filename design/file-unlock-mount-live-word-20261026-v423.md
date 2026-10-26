# Proposal B v423: preserve packed descriptor during mount unlock

Status: accepted one-word resident reduction on the full linked image.

`file_unlock_mount` previously extracted the six-bit mount ID into AC4,
then reread the unchanged descriptor from its table after comparison.
The new implementation extracts into scratch AC5, retaining the full
packed descriptor in AC4 and eliminating the redundant memory load.
The full six-bit mount comparison and regular/lock-state clearing are
unchanged; no new storage or API change is introduced.

Full linked PDP-6 lowmem GCC measurement:

                        before v423      v423      saved
KCORE                       8999         8998         1
KCORE+MRES                 15547        15546         1
PERMANENT_LAST            15594        15593         1

`daimos-testkit` native `target-file-pdp6` PASS and
`test-daimos-file-directmount-20261026-v417` PASS (mounts 2/26/63,
FLOCK, DUP/DUP2, close and unmount, PIPE/TSFS identity, UMASK9 and
old-format source guard). Unrelated testkit changes were untouched.
Physical process u-area allocation remains unchanged. Strict 32K
acceptance and JRST folding remain deferred.
