# K15 v1981: staged slot-zero directory restore service

Parent dd65001b. This closes only the source-linked service subgate.
K15 is still open; no live mode3 build or fixed KCORE change.

The existing scheduler candidate tests use absent APR and backing
record, not the physical representation. The PURE service reads the
same two-bit storage cell as its historical v1667 stage. Under the
new restore flag, mode3 dispatches v1980 directory FULL, mode2 keeps
original RAW PREFIX/FULL. Reject simultaneous old interleaved v1667.
Reuse existing TRANSITION, publication-derived READY_ONCE and cooldown.
A lost successful restore reply does not revoke READY_ONCE. Neither
clean refusal nor dirty escrow is runnable. Clean unlock shares K14
source geometry discovery, exact MM owner proof and u-area checks.
Under PI exclusion it requires mode3, slot-zero inactive TRANSITION,
VM0, valid saved backing identity, marked eligibility, zero escrow,
no active map/fatal close, and exact sole unpinned bitmap. Backing
availability is not required for retry. Do not touch physical payload
or retire pins on unlock. Only the exact clean inactive owner can
be selected again. Do not arm old RAW recovery for dirty directory
transactions; explicit old RAW recovery requests reject combined kind.
K16 owns deferred ambiguous transactions.

Matched KCC -O PDP6 DOBJ: FULL1785->1904 (+119); service901->955
(+54); combined staged incremental2180 (+173). Zero DATA/BSS.
Both disabled objects byte-identical to parent. No new fixed KCORE
word or persistent per-slot field. No installed build flag enabled.

Testkit2c48581. Evidence relative to TMPDIR:

- pure-bitmap-service-v1981.eLvFjy: PDP6/KA10 each46 service/unlock
  operations. Complete production service and RAW recovery functions
  linked, actual v1979 DROP/v1980 FULL, real MM/PI and bounded loader.
  Controlled request/VFS/fault fixtures, not actual scheduler or PI6.
  Plain/compressed backing, lost successful restore reply, one-shot
  READY_ONCE, clean partial read/no-fit/pin/commit/mount refusal then
  retry, dirty transaction refuses old recovery and new request,
  seven request admission and eight clean-unlock refusals. Five
  executed unsafe variants rejected (octal2,10,16,16,2): trust reply,
  bypass unlock proof, ignore escrow, ignore owner, disable mode3.
  Deliberate mutants have expected unused-variable/function notes.
- pure-bitmap-full-v1980.RaNDxM: prior52 calls each CPU and six
  unsafe variants PASS after sharing source proof.
- k15-service-v1981.aJVPtd: matched prior/off/on DOBJ comparisons.
  Candidate and off objects compile without new notes.

Remaining K15: terminal exact owner and original length retry identity,
common proc_swap_detach/VM destroy, actual RUN/EXEC/EXIT/WAIT and
scheduler PI6/READY_ONCE acceptance. Fixed vm_space_destroy currently
refuses mode3 before dispatching its existing terminal trampoline;
keep this refusal until safe terminal integration is proved. Existing
RAW PREVM retains original sector source in word067 LH, while RH is
saved exit status. Variable bitmap span cannot persist in that RH.
Investigate lifetime-disjoint existing u-area fields for exact span;
do not repurpose live backing identity or adopt a new owner after
refusal. No terminal policy selected or implemented in this subgate.
K16 dirty recovery, physical acceptance, PI latency/stack, net RAM
remain open. No default activation.
