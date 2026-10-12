# SUPERVISOR stack and 002000-word resident-core target — v5

Date: 2026-10-12. Source basis: PDP-6 KCC output for SUPERVISOR v4,
`system/kernel/proc/exec.h`, `vm_pdp6.c`, `proc_run.s`, and the native
LOGCTL syscall dispatch. All counts below are **octal 36-bit words**.

## Verified current allocation

| Item | Words |
| --- | ---: |
| Supervisor executable text | 001442 |
| Supervisor static BSS | 000261 |
| Executable origin | 000020 |
| Subtotal before user stack | **001743** |
| General executable stack reservation | 002000 |
| Total before VM rounding | **003743** |
| Rounded resident VM extent | **004000** |

Only **000035 octal words** remain below the 002000-word allocation
boundary before accounting for **any** stack. Thus reducing the default
stack alone cannot get this executable below 002000: the present
logging routine requires more stack than the available margin.

## PDP-6 generated stack frames

The KCC-produced `supervisor.s` currently begins `spawn_entry` with
two `PUSH` saves followed by `ADD 17,[072,,072]`: at least **0074**
octal words of local frame/saves at the deepest child-launch point.
`read_entry` saves four words and reserves five more. The main loop
saves three words and reserves another three.

With `DAIMOS_ULOG_STACK_RECORD`, `ulog_event` begins
`ADD 17,[0205,,0205]`: at least **0205 octal** words for its own
frame, not counting the caller, syscall frame, or any interrupted state.
The large frame comes from the temporary `0200`-word LOGSTORE record.

These are **static lower bounds**, not an observed high-water mark.
Target-side watermark/guard instrumentation must measure the maximum
stack footprint during startup, child launch, repeated respawn, log
failures and error handling before reducing a stack reservation.

## Why the log record cannot simply be shortened

`native_sys_logctl_append` explicitly maps **0200 user words** before
calling the existing LOGSTORE append service. A shorter record would
violate the syscall's mapped-memory contract. A dedicated scalar-event
syscall could conceivably construct the record on the kernel side, but
that adds kernel code and must be evaluated against its own permanent
KCORE and security/validation cost. Do not disguise this ABI change as
a local buffer optimization.

## Necessary savings for a 002000-word SUPERVISOR

If a tested per-image stack mechanism supplied a reservation of:

| Hypothetical stack | Combined reduction in text/BSS required |
| --- | ---: |
| 001000 | 000743 words |
| 000400 | 000343 words |
| 000200 | 000143 words |

The 000200 option is **already smaller than the logged routine's own
0205 frame** and therefore unsafe without changing the logging design.
Other callers increase the required stack further. The 000400 option
is a potential research target only after concrete stack testing and
substantial additional text/BSS reduction.

## Next steps

1. Add **testkit-only** user-stack high-water and guard instrumentation
   without changing production loader defaults.
2. Exercise all eight INITTAB slots, ONCE/RESPAWN, repeated log writes,
   error paths, and resource exhaustion; record worst-case stack.
3. Design and test a per-executable stack-size declaration in DXR only
   after its ABI and startup-argument placement are fully specified.
4. Compare new kernel costs to savings; do not sacrifice real-time,
   memory safety, stack integrity, logging, or shell behavior to reach
   the numerical 002000 boundary.

No production code or stack reservation has been changed by this audit.
