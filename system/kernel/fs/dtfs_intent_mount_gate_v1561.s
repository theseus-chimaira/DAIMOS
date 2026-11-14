; v1561: bind an ordinary DTFS DATA/DIR WRITE to its retained MM owner.
;
; The common DTFS veneer receives only AC1=physical DTC unit.  A V2
; mount must be unique (v1557), and its first-write service needs the
; actual zero-based VFS slot plus the stable MM scratch basep (v1560).
; The caller holds fs_provider_reg_call's exclusive provider lease.
;
; AC1=physical DTC unit. Returns 0 for legacy (no V2 on this unit),
; or after a physically verified DTVOL2/DIRTY barrier; -1 otherwise.
; AC2/AC3 are caller scratch: dtfs_dtc_write saves block/buffer around
; this call. AC10..AC16 are preserved. No permanent KCORE/MRES state.
;
; IMPORTANT: in production v1555's earlier READ-ONLY V2 guard always
; rejects V2 before this routine can arm an RBN0. This code is linked
; and called for ABI/integration coverage, but V2 RW remains DISABLED
; until an exclusive physical CLEAN unmount and full-guest fault tests.

        .text
        .globl dtfs_intent_mount_gate_v1561
        .globl dtfs_intent_first_write_v1553
        .globl vfs_mount_root_base
        .globl dtfs_media_base
        .globl vfs_mount_aux_base_v1560
        .globl vfs_mount_limit
        .globl vfs_mount_policy

dtfs_intent_mount_gate_v1561:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                   ; original physical selector
        jumpl   010,dtfs_mount_gate_refuse_v1561
        caile   010,7
        jrst    dtfs_mount_gate_refuse_v1561
        move    011,vfs_mount_root_base
        jumpe   011,dtfs_mount_gate_refuse_v1561
        move    012,dtfs_media_base
        jumpe   012,dtfs_mount_gate_refuse_v1561
        move    014,vfs_mount_limit
        jumple  014,dtfs_mount_gate_refuse_v1561
        setz    015,                    ; scanning zero-based VFS slot
        setz    016,                    ; matching V2 slot+1, or zero
        setz    7,                      ; all published DTFS owners of unit
dtfs_mount_gate_next_v1561:
        move    4,(011)
        ldb     5,[POINT 6,4,5]
        caie    5,5                     ; only published DTFS roots
        jrst    dtfs_mount_gate_skip_v1561
        move    5,(012)
        move    6,5
        andi    6,7
        came    6,010
        jrst    dtfs_mount_gate_skip_v1561
        aoj     7,                      ; physical unit owner count
        trnn    5,0100000              ; only V2 needs marker
        jrst    dtfs_mount_gate_skip_v1561
        jumpn   016,dtfs_mount_gate_refuse_v1561
        ldb     6,[POINT 6,4,11]       ; authenticated public mount id
        movei   2,1(015)
        came    6,2
        jrst    dtfs_mount_gate_refuse_v1561
        move    016,2                   ; original V2 slot+1
dtfs_mount_gate_skip_v1561:
        aoj     011,
        aoj     012,
        aoj     015,
        sojg    014,dtfs_mount_gate_next_v1561
        jumpe   016,dtfs_mount_gate_allowed_v1561 ; old V1/V1 allowed
        caie    7,1                     ; V2 requires ONE unit owner
        jrst    dtfs_mount_gate_refuse_v1561

        ; Snapshot original published root and expected post-arm media.
        ; v1553 itself compares both saved 36-bit volume-ID halves and
        ; the original media word after sleeping Type-551 I/O.
        move    4,016
        subi    4,1
        move    014,vfs_mount_root_base
        add     014,4
        move    014,(014)
        move    015,dtfs_media_base
        add     015,4
        move    015,(015)
        iori    015,0040000            ; only ARMED may be newly set

        ; No physical RBN0 DIRTY write may occur on an RO VFS mount,
        ; even if this private bridge is mistakenly invoked directly.
        move    5,4
        movn    5,5
        move    6,vfs_mount_policy
        lsh     6,0(5)
        trne    6,1
        jrst    dtfs_mount_gate_refuse_v1561
        move    013,vfs_mount_aux_base_v1560
        jumpe   013,dtfs_mount_gate_refuse_v1561
        add     013,4                   ; EXACT retained per-mount basep
        skipn   (013)
        jrst    dtfs_mount_gate_call_v1561
        jrst    dtfs_mount_gate_refuse_v1561

dtfs_mount_gate_call_v1561:
        move    1,016                   ; authenticated public slot+1
        subi    1,1                     ; exact zero-based VFS slot
        move    2,013                   ; &MM-backed aux[slot]
        pushj   17,dtfs_intent_first_write_v1553
        jumpn   1,dtfs_mount_gate_refuse_v1561

        ; All I/O above may sleep. Never authorize a DATA/DIR transfer
        ; after the slot, policy or exact owner changed in the interval.
        skipn   (013)
        jrst    dtfs_mount_gate_post_owner_v1561
        jrst    dtfs_mount_gate_refuse_v1561
dtfs_mount_gate_post_owner_v1561:
        move    4,vfs_mount_root_base
        jumpe   4,dtfs_mount_gate_refuse_v1561
        move    5,016
        subi    5,1
        add     4,5
        move    6,(4)
        came    6,014
        jrst    dtfs_mount_gate_refuse_v1561
        move    4,dtfs_media_base
        jumpe   4,dtfs_mount_gate_refuse_v1561
        add     4,5
        move    6,(4)
        came    6,015
        jrst    dtfs_mount_gate_refuse_v1561
        movn    5,5
        move    6,vfs_mount_policy
        lsh     6,0(5)
        trne    6,1
        jrst    dtfs_mount_gate_refuse_v1561
dtfs_mount_gate_allowed_v1561:
        setz    1,
        jrst    dtfs_mount_gate_done_v1561
dtfs_mount_gate_refuse_v1561:
        seto    1,
dtfs_mount_gate_done_v1561:
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
