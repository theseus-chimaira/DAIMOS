# Proposal B v413: approved migration, verified baseline, first test

Status: both design decisions approved by user. Implementation IN PROGRESS.

## Explicit approvals

1. FLOCK becomes process-local and vnode-wide, replacing per-OPEN
   family identity. No interprocess exclusion guarantee is introduced.
2. FILE provider and local-kind values are limited to 0..7. Generic
   VFS compatibility for wider providers is not implied by this layout.

## Fresh, full-linked reference (2026-10-26)

Executed `make -s permanent-size PDP10_PREFIX=$HOME/cross
COMPILER=gcc BOOT_PROFILE=lowmem` against current unmodified
production checkout. The build completed successfully:

    KCORE             9109 words
    KCORE+MRES       15657 words
    PERMANENT_LAST   15704 words
    SINGLE_ROOT headroom 679 words
    MULTI_ROOT headroom 646 words

These are pre-migration numbers, NOT results of the new layout.

## First committed regression

The separately versioned host-only C99 test in daimos-testkit,
`tests/host/daimos-file-layout-20261026-v413.c`, checks that the
new six metadata bits do not overlap the restricted vnode mask,
and exhausts 64 mount IDs times four lock-state representations.
Executed using host C99 compiler with `-Os -Wall -Wextra -Werror`;
PASS. This is NOT a PDP-6 assembly/runtime regression.

The daimos-testkit checkout already contained unrelated modifications;
none were staged or committed by this task.

## Atomic production conversion checklist

Before modifying FILE runtime, provide linked test coverage for:

* Replacement of vnode extension reconstruction in all consumers.
* Boot and C OPEN, FIFO/PIPE, FLOCK, CLOSE, SEEK, READ/WRITE, READDIR.
* RUN and syscall dispatch paths, PIPE refcount identity with TSFS.
* Provider/local-kind boundary enforcement and full mount bits.
* Close failures and RUN construction ownership ordering.

Do not claim the bitmap word as physically freed without safely
shrinking the actual process u-area allocation. Reject any full-link
or total-RAM regression. Strict 32K acceptance and JRST folding remain
deferred.
