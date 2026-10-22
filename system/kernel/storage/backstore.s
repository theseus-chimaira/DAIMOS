; backstore.s -- fixed KCORE backing-store ABI, state, and boot initializer.
;
; Process swap and MEMFS share one direct in-RAM allocation bitmap. One bit
; represents one 0200-word backing block; set means allocated. The first-fit
; allocator loops now live in the pinned backing allocator MRES, while KCORE
; retains the permanent public jump slots and the four-word state block.
; The only state owner is KCORE; no duplicated bitmap or accounting words.
;
; The implementation uses only the common PDP-6/PDP-10 instruction subset.
; Backstore sizes are bounded non-negative block counts, so normal signed
; compare instructions are sufficient and avoid generic unsigned helpers.

        .text
        .globl  backstore_bitmap_words
        .globl  backstore_init
        .globl  backstore_alloc
        .globl  backstore_free

; unsigned int backstore_bitmap_words(kword_t blocks)
backstore_bitmap_words:
        addi    1,043                  ; ceil(blocks / 36)
        idivi   1,044
        popj    17,

; void backstore_init(kword_t *bitmap, kword_t blocks)
backstore_init:
        movem   1,backstore_bitmap
        movem   2,backstore_blocks
        setzm   backstore_blocks_used
        setzm   backstore_enabled
        popj    17,

; KCORE ABI: MINIT patches these two one-word, stable-address JRSTs.
; Before the pinned allocator is installed, allocations fail safely and
; frees are inert. Backstore bitmap initialization remains available to
; KINIT without depending on any MRES export address.
        .globl kret_neg1
backstore_alloc:
        jrst    kret_neg1
backstore_free:
        popj    17,

        .bss
        .globl  backstore_bitmap
        .globl  backstore_blocks
        .globl  backstore_blocks_used
        .globl  backstore_enabled
backstore_bitmap:       .block 1
backstore_blocks:       .block 1
backstore_blocks_used:  .block 1
backstore_enabled:      .block 1
