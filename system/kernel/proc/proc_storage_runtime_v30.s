; PDP-6 resident packed two-bit storage-mode checks for disk swap.
; Caller serializes publication under PROC_TRANSITION. AC4..AC7 scratch.
; AC1=slot, AC2=expected, AC3=new state for publish. Returns 0 or -1.
        .text
        .globl  proc_storage_runtime_v29
        .globl  proc_storage_expect_v30
        .globl  proc_storage_publish_v30
        .globl  proc_storage_exchange_modes_v778

proc_storage_expect_v30:
        skipn   4,proc_storage_runtime_v29
        jrst    proc_storage_bad_v30
        jumpe   1,proc_storage_bad_v30
        caml    1,proc_storage_runtime_v29+1
        jrst    proc_storage_bad_v30
        caile   2,2
        jrst    proc_storage_bad_v30
        move    4,1
        idivi   4,022                   ; 18 slots per 36-bit word
        lsh     5,1                     ; two bits per slot
        add     4,proc_storage_runtime_v29
        movn    5,5
        move    6,(4)
        lsh     6,0(5)
        andi    6,3
        camn    6,2
        jrst    proc_storage_good_v30
proc_storage_bad_v30:
        seto    1,
        popj    17,
proc_storage_good_v30:
        setz    1,
        popj    17,

proc_storage_publish_v30:
        caile   3,2
        jrst    proc_storage_bad_v30
        push    17,1                   ; retain slot across expectation
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_storage_publish_fail_v30
        move    4,(17)
        idivi   4,022
        lsh     5,1
        add     4,proc_storage_runtime_v29
        movei   6,3
        lsh     6,0(5)
        andcam  6,(4)                  ; clear the old two-bit mode
        lsh     3,0(5)
        iorm    3,(4)                  ; publish new mode
        sub     17,[1,,1]
        setz    1,
        popj    17,
proc_storage_publish_fail_v30:
        sub     17,[1,,1]
        seto    1,
        popj    17,

; int proc_storage_exchange_modes_v778(a_slot,b_slot)
; Change RESIDENT A (0) to DISK (1), and DISK B (1) to RESIDENT (0).
; Caller holds both PROC_TRANSITION locks and must have published both
; VM and backing records. AC10/AC11 preserve the slot IDs across calls.
; Both expected modes are validated before either bit pair changes.
proc_storage_exchange_modes_v778:
        jumpe   1,proc_storage_exchange_invalid_v778
        jumpe   2,proc_storage_exchange_invalid_v778
        camn    1,2
        jrst    proc_storage_exchange_invalid_v778
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        move    1,010
        setz    2,
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_storage_exchange_fail_v778
        move    1,011
        movei   2,1
        pushj   17,proc_storage_expect_v30
        jumpn   1,proc_storage_exchange_fail_v778
        move    1,010
        movei   2,0
        movei   3,1
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_storage_exchange_invariant_v778
        move    1,011
        movei   2,1
        setz    3,
        pushj   17,proc_storage_publish_v30
        jumpn   1,proc_storage_exchange_invariant_v778
        setz    1,
        jrst    proc_storage_exchange_return_v778
proc_storage_exchange_invariant_v778:
        ; Unexpected mutation under lifecycle exclusion: do not resume.
        halt    .
proc_storage_exchange_fail_v778:
        seto    1,
proc_storage_exchange_return_v778:
        pop     17,011
        pop     17,010
        popj    17,
proc_storage_exchange_invalid_v778:
        seto    1,
        popj    17,

; KCORE v1538: fail-stop for an exhausted physical I/O on the selected root.
; AC1=filesystem provider (6 D6FS, 7 TSFS), AC2=public mount ID.
; The calling provider supplies these from the actual failed I/O owner,
; never from the physical controller number alone.  A secondary mount,
; failed boot probe before root publication, or unrelated provider must
; return -1 through the normal filesystem error path.  This entry is
; only called for a failed backing/physical transfer on a previously
; validated filesystem operation. A missing backing callback is itself an
; unusable root provider; a malformed user argument or EOF never enters
; this path. No persistent device or fault table is needed.
        .globl  kcore_root_fs_io_failure_v1538
        .globl  vfs_namespace_root
kcore_root_fs_io_failure_v1538:
        lsh     1,6                     ; provider,,public mount ID
        ior     1,2
        move    2,vfs_namespace_root
        lsh     2,-030                  ; compare root's top twelve bits
        camn    1,2
        jrst    kcore_root_fs_io_halt_v1538
        seto    1,                      ; secondary: retain normal I/O error
        popj    17,
kcore_root_fs_io_halt_v1538:
        movei   1,1                     ; root-specific fatal reason
        jrst    kcore_critical_device_lost_v1515

; KCORE v1515: terminal dispatch after a *critical service* has failed.
; AC1=1: selected boot ROOT; AC1=2: active or still-owned BACKSTORE.
; Unknown reason is a caller invariant violation, with its own HALT PC.
; Never enter directly from a controller PI/watchdog: those events may be
; unrelated to the critical service, or the driver may recover internally.
; v1538 ROOT and v1541 BACKSTORE caller gates classify an actual failed,
; validated I/O after normal bounded driver recovery. No proof of permanent
; physical media disappearance is required for such a critical request;
; identical failures on unrelated secondary filesystem mounts are nonfatal.
;
; No KINIT, console, process-private stack or live filesystem is needed here.
; Distinct HALT *addresses* identify the failed owner without storing a
; panic string or another permanent data word. HALT . leaves the halted PC
; at the actual instruction, never in low-core vector words; CONTINUE just
; halts again. There is no cleanup or owner release.
        .globl  kcore_critical_device_lost_v1515
kcore_critical_device_lost_v1515:
        cono    0004,000400             ; disable global PI before HALT
        caie    1,1
        jrst    kcore_critical_media_check_swap_v1515
kcore_critical_media_root_halt_v1515:
        halt    .                       ; confirmed boot root lost
        jrst    .-1
kcore_critical_media_check_swap_v1515:
        caie    1,2
        jrst    kcore_critical_media_bad_reason_v1515
kcore_critical_media_swap_halt_v1515:
        halt    .                       ; confirmed BACKSTORE lost
        jrst    .-1
kcore_critical_media_bad_reason_v1515:
        halt    .                       ; invalid caller reason
        jrst    .-1
