# KCORE PREVM teardown after refused backing release, v1494

Date: 2026-11-11. Feature correctness work only.

## Production path inspected

The scheduler service on a negative swap-in result calls
`proc_event_apply(slot, SYS_EVENT_TERM)`. The production fatal-event
handler in `proc/proc_pdp6.s` either defers delivery to an owned user
continuation or calls `proc_finish_slot` after closing the target's
FILEs. The finalizer enters `vm_space_destroy`, which calls checked
`proc_swap_detach` for a nonresident DISK owner. An incomplete
BACKSTORE bitmap makes this operation fail without clearing the disk
record or decrementing usage. A VM teardown failure is published as
FINALIZE+PREVM with TRANSITION still set. The slot-0
`proc_finish_service_v881` retries these marked non-sleeping owners;
it must neither repeat sleeping HUP nor free a process-owned u-area
before its backing is released.

## Native evidence

daimos-testkit v1494 links the actual `proc_finish_slot` and
`proc_finish_service_v881` production leaves. The controlled VM
backend rejects the first VM destroy, then accepts it. Two additional
u-area MM refusals exercise the post-VM retry stage. PASS:

- the original VM owner and u-area survive the first refusal;
- FINALIZE+PREVM retains the exit status and arms deferred cleanup;
- idle retry releases the VM, then u-area retry completes once;
- HUP does not recur in the PI6-masked idle finalizer;
- VM is not destroyed again after committed release;
- premature-commit negative mutation is rejected.

Existing v1081 refused-TTY PREVM and v1086 PI6-masked HUP avoidance
regressions also PASS. No new production code, ABI or resident RAM.

## Still open

This native test uses a *controlled* VM destroy result and does not
source-link physical `vm_space_destroy`/`proc_swap_detach`/BACKSTORE
bitmap release into the real finalizer. v1493 source-links real
swap-in + bitmap release through the slot-0 service, but observes
TERM with a stub. The complete cross-boundary integration, especially
real 256K guest corrupted-bitmap injection followed by actual TERM,
PREVM finalizer retries and exact eventual backing owner resolution,
remains OPEN. A permanently corrupt bitmap must never be reported as
released. Accepted normal v1405/v1466 physical swap tests do not
discharge this special failure test.
