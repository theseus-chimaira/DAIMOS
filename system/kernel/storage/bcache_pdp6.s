; bcache_pdp6.s -- tiny shared clean two-block RAM cache.
;
; The cache stores only regenerable/write-through blocks.  One managed-core
; 0403-word slab contains an alternating victim hand, two 36-bit tags, and two
; 0200-word data blocks.  There is no dirty state, LRU list, or writeback path.
        .text
        .globl  bcache_fetch
        .globl  bcache_store
        .globl  bcache_reclaim
        .globl  mm_alloc_aligned_noreclaim
        .globl  mm_free
        .globl  kret_zero
        .globl  kret_one

        .equ    BCACHE_SLAB_WORDS,0403
        .equ    BCACHE_MM_OWNER,6

; int bcache_fetch(key, buf)
bcache_fetch:
        jumpe   2,kret_zero
        hrrz    3,bcache_state
        jumpe   3,kret_zero
        camn    1,1(3)
        jrst    bcache_fetch_slot0
        came    1,2(3)
        jrst    kret_zero
        movei   5,0203(3)
        jrst    bcache_fetch_copy
bcache_fetch_slot0:
        movei   5,3(3)
bcache_fetch_copy:
        move    4,2
        hrl     4,5
        move    6,2
        addi    6,0177
        blt     4,(6)
        jrst    kret_one

; void bcache_store(key, buf)
bcache_store:
        jumpe   2,bcache_store_done
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        hrrz    3,bcache_state
        jumpn   3,bcache_store_scan

; Allocate without invoking reclaim, otherwise cache allocation could recurse
; through fs_memory_reclaim().  The third stack word is the returned base.
        add     17,[3,,3]
        setzm   -2(17)
        movei   1,-2(17)
        movem   1,-1(17)               ; arg 6: basep
        movei   1,1
        movem   1,(17)                  ; arg 5: MM_ALLOC_HIGH
        movei   1,BCACHE_SLAB_WORDS
        movei   2,1
        movei   3,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   4,BCACHE_MM_OWNER
        pushj   17,mm_alloc_aligned_noreclaim
        jumpn   1,bcache_store_alloc_fail
        move    3,-2(17)
        jumpe   3,bcache_store_alloc_fail
        sub     17,[3,,3]
        setzm   (3)                     ; alternating victim hand
        setom   1(3)                    ; invalid tag 0
        setom   2(3)                    ; invalid tag 1
        hrrm    3,bcache_state
        jrst    bcache_store_scan

bcache_store_alloc_fail:
        sub     17,[3,,3]
        jrst    bcache_store_restore

bcache_store_scan:
        camn    010,1(3)
        jrst    bcache_store_slot0
        camn    010,2(3)
        jrst    bcache_store_slot1
        move    4,(3)
        xori    4,1
        andi    4,1
        movem   4,(3)
        jumpn   4,bcache_store_slot1

bcache_store_slot0:
        movem   010,1(3)
        movei   5,3(3)
        jrst    bcache_store_copy
bcache_store_slot1:
        movem   010,2(3)
        movei   5,0203(3)

bcache_store_copy:
        hrl     5,011
        move    6,5
        addi    6,0177
        blt     5,(6)

bcache_store_restore:
        pop     17,011
        pop     17,010
bcache_store_done:
        popj    17,

; int bcache_reclaim(words)
; The requested count is irrelevant: the sole clean slab is all-or-nothing.
bcache_reclaim:
        push    17,010
        hrrz    010,bcache_state
        jumpe   010,bcache_reclaim_none
        move    1,010
        movei   2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,BCACHE_MM_OWNER
        pushj   17,mm_free
        jumpn   1,bcache_reclaim_none
        setzm   bcache_state
        movei   1,1
        pop     17,010
        popj    17,
bcache_reclaim_none:
        setz    1,
        pop     17,010
        popj    17,

        .bss
bcache_state:    .block 1
        .text
