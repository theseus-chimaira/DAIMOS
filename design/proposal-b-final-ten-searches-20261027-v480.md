# Proposal B v480: final ten small-saving searches

Ten resident-source regions were reviewed following v479. These are
source-level searches, not ten linked experiments. No source changed.

1. FILE OPEN: lookup/create, FIFO and regular publication have separate
   ownership; common success publication already exists.
2. FILE CLOSE: DPY, PIPE and VFS need distinct cleanup and failure handling.
3. FILE SEEK: SET/CUR share storage; END requires a separate STAT frame.
4. FILE descriptor allocation: one compact 16-slot scan and publication.
5. FILE removal: UNLINK/RMDIR share policy and provider path; FIFO detach
   is conditional.
6. FILE SYMLINK: target validation and parent frame already use common
   cleanup.
7. VFS mutations: CHMOD/CHOWN/UTIME share readonly/provider continuation.
8. VFS WRITE/TRUNCATE: readonly/swap checks and unwind already shared.
9. VFS MOUNT: unique slot selection and publication require distinct
   correctness boundaries.
10. VFS UNMOUNT: busy/sync/prepare/unlock/retire phases have different
    rollback and ownership conditions.

No credible safe candidate above two linked words was identified in
these searches. This is not proof no other optimizations exist.
Earlier rejected cross-layer rewrite experiments remain rejected until
they demonstrate a measured full-linked RAM benefit.

Focused native regression runs on the current source:
- VFS SYMLINK v448: PASS
- VFS RENAME v455: PASS
- FILE RENAME v457: PASS

Actual userland end-to-end SYMLINK and a real PI6 context switch
inside a sleeping RENAME remain unverified; these are future targeted
acceptance coverage, not validated failures.

Proposal B optimization search is closed. Last measured baseline:
KCORE 8960, KCORE+MRES 15507, PERMANENT_LAST 15554 words.
Strict 32K P0 remains open; the default linked KCORE requires 769
more words saved to be strictly below 8192.
