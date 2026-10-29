# Native PDP-6 post-write MM-free rollback v668

Testkit v668 executes the actual production PDP-6 swap-out segment
from its post-write u-area staging through the failed `mm_free` branch.
`mm_unpin` succeeds, `mm_free` returns an error, and the test checks
that the original swap record and resident VM base remain unchanged;
TRANSITION and the provisional u-area backing word are cleared, while
the provisional BACKING span is freed. The isolated native test PASSes.

This completes instruction-level coverage of the recoverable late
`mm_free` failure path, in addition to v667's fatal `mm_unpin` failures.
It is not a full scheduler-level injection or a controller-recovery
test. No production code, disk format, or RAM optimization changed.
