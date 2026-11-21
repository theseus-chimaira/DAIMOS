# K04 v1793: physical SIDL A / PREVM B independent owners

Date: 2026-11-21. Status: **PASS** for SIDL A followed by EXIT PREVM B,
with independent physical MM ownership, capped-A/fresh-B retry,
real MM release, PID reuse and clean shutdown. Overall K04 remains OPEN.

## Physical scope

The previous v1785 guest validated unpublished SIDL A followed by
post-VM B. This test changes B to an EXIT PREVM owner. A (PID 5)
fails its first RUN at the existing hierarchy rollback, pins its
exact VM extent through 12 actual `vm_space_destroy` calls and
remains unpublished SIDL. B (PID 6) uses the original EXIT PREVM
path and pins a distinct VM extent through its first four real
destroy attempts. Its fifth attempt performs real `mm_unpin` and
the ordinary VM destroy completes. A subsequently unpins and
finishes its normal deferred SIDL discard.

The generated scratch fixture is based on the proven v1785 SIDL
and v1783 PREVM assembly, with no fake MM error, no production
source edit, no scheduler policy override and no fixed KCORE size
or ABI change. The corrected generator is in `daimos-testkit`:

    tests/validation/daimos-k04-sidl-prevm-fixture-v1793.py

Its two inputs are the generated v1785 `kcore-asm` tree (to be
modified) and the generated v1783 `kcore-asm` tree (read-only).
The generator's output was replayed in a separate scratch directory
and compared byte-for-byte against the final tested generated
`proc_pdp6.s` and `proc_run.s`: both matched. The inherited scratch
symbol names retain suffix `v1788`; the accepted image is v1793.

The final disposable 256K image is located under:

    $TMPDIR/daimos-k04-v1793-sidl-prevm-corrected

Native DAS, full image build, initial read-only `d6fsck -n 4`
(superblock 742, sequence 1, CLEAN) passed. The simulator ran
at reduced priority with only CTY input; unused DCS/GE sockets
were disabled in the scratch `boot.ini`. This avoided interfering
with the independently running `pdp10-testkit` sweep.

## Real PDP-6 console and memory evidence

Log in to CTY as ROOT, issue `TRUE` to create the unpublished
SIDL owner A, then issue a second `TRUE` while A remains pinned.
The second process becomes B in EXIT PREVM. After B's fifth
physical VM destroy attempt, CTY `PS` reports:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    1 2048  3 256   0   1027   0 TRUE
    6   2    2 3072  0   0   0      0   0 PS

Thus A remains unpublished and owned (stage 3) while B's slot is
already reusable with no teardown diagnostics. The shared delay
may be rearmed by B and then advance again for A; its intermediate
PS value is not expected to remain at the previous maximum.

After the deferred A recovery, CTY `PS` reused PID 5 with all
teardown diagnostics zero. Guest `HALT` completed normally. The
scratch SIMH command file examined memory only **after** HALT:

    025126: 000000000000  ; A exact SIDL VM base released
    025127: 000000000014  ; A twelve real VM destroy attempts
    025130: 777777777777  ; A real mm_unpin completed
    025131: 000000000000  ; B exact PREVM VM base released
    025132: 000000000005  ; B five real VM destroy attempts
    025133: 777777777777  ; B real mm_unpin completed
    025134: 401000003446  ; BEFORE B: A capped 01000 retry
    025135: 400020004140  ; AFTER B: fresh pending 020 retry
    025136: 000000004140  ; B first real refusal clock
    025137: 000000004160  ; B second real destroy attempt clock

The first B interval is octal `004160 - 004140 = 020`, exactly
16 qualified ticks. Its initial delay is independent of A's
capped interval. A and B both release their exact physical MM
owners and neither is prematurely reused. Final read-only
`d6fsck -n 4` selected superblock 742, sequence 3, CLEAN,
179196 blocks and 360 FCBs.

The read-only verifier in `daimos-testkit` is:

    tests/validation/daimos-k04-sidl-prevm-verify-v1793.py

It accepted the actual physical CTY and SIMH transcripts, symbol
map and CLEAN disk, and passed its positive and six negative
controls. The fixture generator's byte-identical replay also
passed.

## Debugging the earlier disposable assembly defect

The initial v1788 fixture produced a repeatable `UUO` trap when
the second TRUE reached PREVM cleanup. This was not accepted as a
kernel regression. Two independent guest-terminal arrangements
reproduced it, so the terminal-control sequence was not the cause.

Full instruction history from the disposable v1792 diagnostic
image exposed a one-word stack imbalance. The source generator
had removed the existing `popj 17,` after
`proc_finish_mark_pending_v892` while stripping obsolete v1785
post-VM B instrumentation. Consequently the helper fell through
to `proc_finish_fail` without popping its own return address;
`proc_finish_restore` restored the wrong stack frame and returned
to address zero. The resulting user UUO and kernel HALT were
fixture errors, not evidence of a production KCORE defect.

The corrected v1793 generator preserves the mandatory `popj 17,`.
Native DAS and the physical guest passed afterwards. The rejected
crash images and their DIRTY D6FS volumes were never counted as
acceptance; fresh CLEAN media was used for the successful run.
No production assembler or kernel source was changed.

## Remaining K04 physical coverage

This closes **SIDL A / PREVM B** only. Reverse **PREVM A / SIDL B**,
additional mixed-owner clock-wrap stress, FILE/HUP provider
refusal, dying-parent WAIT/PID reuse, and permanent-refusal
operator recovery remain OPEN. The single-owner SIDL clock-wrap
case separately passed in v1787. Do not mark K04 closed yet.
