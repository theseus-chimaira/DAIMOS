; bcache_pdp6.s -- tiny shared clean two-block RAM cache.
;
; Slot 0 reuses fs_block_workspace, which every resident filesystem already
; serializes.  Its tag is one fixed KCORE word; managed core therefore needs
; only the slot-1 tag plus data: 0201 words instead of the old 0403-word
; private slab.  A workspace-backed
; miss preserves a valid slot-0 block into slot 1 before the caller overwrites
; the workspace.  Direct scratch users invalidate slot 0 explicitly.
        .text
        .globl  bcache_fetch
        .globl  bcache_store
        .globl  bcache_reclaim
        .globl  bcache_workspace_invalidate
        .globl  fs_block_workspace
        .globl  mm_alloc_aligned_noreclaim
        .globl  mm_free
        .globl  kret_zero
        .globl  kret_one

        .equ    BCACHE_SLAB_WORDS,0201
        .equ    BCACHE_MM_OWNER,6

; int bcache_fetch(key, buf)
bcache_fetch:
        jumpe   2,kret_zero
        hrrz    3,bcache_state
        jumpe   3,kret_zero
        camn    1,bcache_workspace_tag
        jrst    bcache_fetch_slot0
        camn    1,(3)
        jrst    bcache_fetch_slot1

; If the caller is about to refill the shared workspace, preserve its current
; clean cache entry in slot 1 before reporting the miss.
        caie    2,fs_block_workspace
        jrst    kret_zero
        move    4,bcache_workspace_tag
        camn    4,[-1]
        jrst    bcache_fetch_miss_ready
        movem   4,(3)
        movei   4,1(3)
        hrli    4,fs_block_workspace
        movei   5,0200(3)
        blt     4,(5)
bcache_fetch_miss_ready:
        setom   bcache_workspace_tag
        jrst    kret_zero

bcache_fetch_slot0:
        caie    2,fs_block_workspace
        jrst    bcache_fetch_slot0_copy
        jrst    kret_one
bcache_fetch_slot0_copy:
        move    4,2
        hrli    4,fs_block_workspace
        move    6,2
        addi    6,0177
        blt     4,(6)
        jrst    kret_one

bcache_fetch_slot1:
        move    4,2
        hrli    4,1(3)
        move    6,2
        addi    6,0177
        blt     4,(6)
        caie    2,fs_block_workspace
        jrst    kret_one
; Copying slot 1 into the shared workspace destroys the old slot-0 contents.
; Promote the requested key and invalidate the now-duplicate slot 1.
        movem   1,bcache_workspace_tag
        setom   (3)
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
; through fs_memory_reclaim().  State is zero here, so let MM write the new
; slab base directly into bcache_state rather than reserving a third local.
        add     17,kconst_2_2
        movei   1,bcache_state
        movem   1,-1(17)               ; arg 6: basep
        movei   1,1
        movem   1,(17)                  ; arg 5: MM_ALLOC_HIGH
        movei   1,BCACHE_SLAB_WORDS
        movei   2,1
        movei   3,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   4,BCACHE_MM_OWNER
        pushj   17,mm_alloc_aligned_noreclaim
        jumpn   1,bcache_store_alloc_fail
        hrrz    3,bcache_state
        jumpe   3,bcache_store_alloc_fail
        sub     17,kconst_2_2
        setom   bcache_workspace_tag    ; invalid workspace tag
        setom   (3)                     ; invalid slot-1 tag
        jrst    bcache_store_scan

bcache_store_alloc_fail:
        sub     17,kconst_2_2
        setzm   bcache_state
        jrst    bcache_store_restore

bcache_store_scan:
        caie    011,fs_block_workspace
        jrst    bcache_store_not_workspace
        movem   010,bcache_workspace_tag
        jrst    bcache_store_restore
bcache_store_not_workspace:
; Arbitrary caller buffers always refresh the private slot, so they never
; overwrite filesystem scratch merely to create a cache entry.  If both tags
; named this key, drop the workspace tag to avoid duplicate ownership.
        came    010,bcache_workspace_tag
        jrst    bcache_store_private
        setom   bcache_workspace_tag
bcache_store_private:
        movem   010,(3)
        movei   5,1(3)

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

; void bcache_workspace_invalidate(void)
; Direct users of fs_block_workspace call this before changing its contents.
; Slot 1 is independent and remains valid.
bcache_workspace_invalidate:
        setom   bcache_workspace_tag
bcache_workspace_invalidate_done:
        popj    17,

        .bss
bcache_state:    .block 1
bcache_workspace_tag: .block 1
        .text
