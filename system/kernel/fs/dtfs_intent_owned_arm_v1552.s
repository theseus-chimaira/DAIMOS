; DTFS v1552: optional MM-owned and physically pinned RBN0 scratch lease.
;
; v1572: actual production V2 RW reaches this owner-checked physical
; first-write service under an explicit trusted-operator no-exchange
; contract. A read-only VFS mount is still rejected by v1561.
;
; int dtfs_intent_owned_arm_v1552(unit, id_high, id_low, basep)
; AC1=physical Type-551 unit, AC2/AC3=previously admitted volume ID,
; AC4=pointer to caller-retained scratch base word, initially zero.
; Return 0 only after successful v1551 dirty arm AND complete MM release.
; Return -1 for any failure. A failure to unpin/free leaves *basep set
; to the exact still-owned MM base for explicit subsequent recovery.
;
; The caller holds the exclusive DTFS provider lease across every step.
; The caller MUST NOT issue a data/directory write when return != 0.
; No fixed KCORE or temporary MRES BSS allocation is needed: the MM
; extent is 0200 words, MM_TYPE_KERNEL_DYNAMIC and owner 015 octal.
; The extent is pinned across the complete physical Type-551 transaction
; so MM compaction cannot relocate the DMA buffer while I/O sleeps.
; v1554 shares the EXACT same pin/owner cleanup for read-only physical
; DTVOL2/DIRTY revalidation, without duplicating the 128-word MM lease.
; v1566 adds exact DIRTY->CLEAN with that SAME pinned 0200-word lease.
; A seal may sleep; before any raw RBN0 I/O, the mounted unit is marked
; WRITE_UNCERTAIN so an interrupted/failed cleanup cannot pass as CLEAN.

        .text
        .globl dtfs_intent_owned_arm_v1552
        .globl dtfs_intent_owned_verify_v1554
        .globl dtfs_intent_owned_clean_v1566
        .globl dtfs_intent_arm_v1551
        .globl dtfs_intent_verify_v1554
        .globl dtfs_intent_clean_v1566
        .globl dtfs_mark_unit_suspect_v1522
        .globl mm_alloc_aligned_noreclaim
        .globl mm_pin
        .globl mm_unpin
        .globl mm_free

        .equ    DTFS_INTENT_SCRATCH_OWNER_V1552,015
        .equ    DTFS_INTENT_SCRATCH_WORDS_V1552,0200
        .equ    MM_TYPE_KERNEL_DYNAMIC_V1552,3

dtfs_intent_owned_arm_v1552:
        setz    5,                      ; arm CLEAN -> DIRTY
        jrst    dtfs_owned_common_v1554
dtfs_intent_owned_verify_v1554:
        movei   5,1                     ; verify already DIRTY
        jrst    dtfs_owned_common_v1554
dtfs_intent_owned_clean_v1566:
        movei   5,2                     ; final DIRTY -> CLEAN
dtfs_owned_common_v1554:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        move    015,5                   ; operation, on private stack
        move    010,1                   ; DTC unit
        move    011,2                   ; expected high ID
        move    012,3                   ; expected low ID
        move    013,4                   ; caller's retained basep
        jumpe   013,dtfs_owned_arm_fail_v1552
        skipn   (013)                   ; prevent overwriting a leaked owner
        jrst    dtfs_owned_arm_alloc_v1552
        jrst    dtfs_owned_arm_fail_v1552

dtfs_owned_arm_alloc_v1552:
        ; Two additional stack arguments, exactly as in the existing
        ; BCACHE no-reclaim allocation. The basep itself is caller-owned.
        add     17,[2,,2]
        move    1,013
        movem   1,-1(17)               ; argument 6 = basep
        movei   1,1
        movem   1,(17)                 ; argument 5 = MM_ALLOC_HIGH
        movei   1,DTFS_INTENT_SCRATCH_WORDS_V1552
        movei   2,1                     ; no alignment overprovisioning
        movei   3,MM_TYPE_KERNEL_DYNAMIC_V1552
        movei   4,DTFS_INTENT_SCRATCH_OWNER_V1552
        pushj   17,mm_alloc_aligned_noreclaim
        sub     17,[2,,2]
        jumpn   1,dtfs_owned_arm_fail_v1552
        move    014,(013)               ; stable physical MM extent base
        jumpe   014,dtfs_owned_arm_fail_v1552

        ; The kernel allocator does not pin new extents automatically.
        ; Pin before the first physical DTC RBN0 READ and keep the pin
        ; until after the final physical RBN0 READBACK has completed.
        move    1,014
        pushj   17,mm_pin
        jumpn   1,dtfs_owned_arm_pin_fail_v1552

        move    1,010
        move    2,011
        move    3,012
        move    4,014
        jumpe   015,dtfs_owned_call_arm_v1554
        caie    015,2
        jrst    dtfs_owned_call_verify_v1554
        ; MM is successfully allocated and physically pinned NOW.
        ; Publish a sticky in-RAM uncertainty marker before any raw
        ; physical seal I/O can sleep or partially complete. If MM
        ; allocation/pinning failed, no I/O started and no poisoning
        ; was necessary. Only the exact mounted seal owner may later
        ; clear this marker after CLEAN readback and MM release.
        move    1,010
        movei   2,0400000
        pushj   17,dtfs_mark_unit_suspect_v1522
        move    1,010
        move    2,011
        move    3,012
        move    4,014
        pushj   17,dtfs_intent_clean_v1566
        jrst    dtfs_owned_after_io_v1554
dtfs_owned_call_verify_v1554:
        pushj   17,dtfs_intent_verify_v1554
        jrst    dtfs_owned_after_io_v1554
dtfs_owned_call_arm_v1554:
        pushj   17,dtfs_intent_arm_v1551
dtfs_owned_after_io_v1554:
        push    17,1                    ; preserve exact arm success/error

        move    1,014
        pushj   17,mm_unpin
        jumpn   1,dtfs_owned_arm_release_fail_v1552
        move    1,014
        movei   2,MM_TYPE_KERNEL_DYNAMIC_V1552
        movei   3,DTFS_INTENT_SCRATCH_OWNER_V1552
        pushj   17,mm_free
        jumpn   1,dtfs_owned_arm_release_fail_v1552
        setzm   (013)                   ; only after confirmed MM free
        pop     17,1                    ; original v1551 arm status
        jrst    dtfs_owned_arm_done_v1552

dtfs_owned_arm_release_fail_v1552:
        ; The MM refusal may still mean a pinned or owned extent.
        ; Do not guess that it was freed. Keep *basep unchanged. CLEAN
        ; already entered the uncertainty state before raw I/O, so a
        ; failed unpin/free cannot accidentally retire the dirty mount.
        pop     17,2                    ; discard original arm status
        jrst    dtfs_owned_arm_fail_v1552

dtfs_owned_arm_pin_fail_v1552:
        ; A failed pin does not create a DMA lease; try to free the
        ; unpinned allocation. If freeing fails, retain *basep.
        move    1,014
        movei   2,MM_TYPE_KERNEL_DYNAMIC_V1552
        movei   3,DTFS_INTENT_SCRATCH_OWNER_V1552
        pushj   17,mm_free
        jumpn   1,dtfs_owned_arm_fail_v1552
        setzm   (013)
dtfs_owned_arm_fail_v1552:
        seto    1,
dtfs_owned_arm_done_v1552:
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
