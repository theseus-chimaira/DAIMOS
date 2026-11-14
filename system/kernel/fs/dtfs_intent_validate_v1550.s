; Native PDP-6 DTVOL2 record validator, installed in DTFS MRES by v1555
; for physical CLEAN/READ-ONLY admission and by v1556 for read-fault
; recovery. The host formatter still emits V2 only on explicit request.
;
; AC1 = pointer to all 0200 physical RBN0 words in kernel memory.
; Return AC1 = 0 CLEAN / 1 WRITE_INTENT / -1 corrupt or unsupported.
; AC2..AC7 scratch; stack, buffers, owners and persistent RAM unchanged.
; V2 DATA/DIR writing remains prohibited until the durable transaction,
; retained MM ownership and exclusive CLEAN/unmount paths are complete.

        .text
        .globl dtfs_intent_validate_v1550
dtfs_intent_validate_v1550:
        jumpe   1,dtfs_intent_bad_v1550
        move    2,(1)
        came    2,[0446466575422]      ; SIXBIT DTVOL2
        jrst    dtfs_intent_bad_v1550
        move    3,1(1)
        jumpe   3,dtfs_intent_bad_v1550
        move    4,2(1)
        jumpe   4,dtfs_intent_bad_v1550
        xor     2,3
        xor     2,4
        setcm   2,2
        came    2,3(1)                 ; full 36-bit identity check
        jrst    dtfs_intent_bad_v1550

        move    5,4(1)
        jumpe   5,dtfs_intent_seal_v1550
        came    5,[0552255225522]      ; exact WRITE_INTENT state
        jrst    dtfs_intent_bad_v1550
dtfs_intent_seal_v1550:
        move    2,(1)
        xor     2,3
        xor     2,4
        xor     2,5
        xor     2,[0246134572610]      ; independent seal salt
        setcm   2,2
        came    2,5(1)
        jrst    dtfs_intent_bad_v1550

        movei   2,6(1)
        movei   6,0172                  ; 122 zero padding words
dtfs_intent_padding_v1550:
        skipn   (2)
        jrst    dtfs_intent_next_v1550
        jrst    dtfs_intent_bad_v1550
dtfs_intent_next_v1550:
        aoj     2,
        sojg    6,dtfs_intent_padding_v1550
        jumpe   5,dtfs_intent_clean_v1550
        movei   1,1
        popj    17,
dtfs_intent_clean_v1550:
        setz    1,
        popj    17,
dtfs_intent_bad_v1550:
        seto    1,
        popj    17,
