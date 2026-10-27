; KCORE ABI and single authoritative backing store state.
        .text
        .globl backstore_bitmap_words
        .globl backstore_init
        .globl backstore_alloc
        .globl backstore_free
        .globl kret_neg1
backstore_bitmap_words: jrst kret_neg1
backstore_init: jrst kret_neg1
backstore_alloc: jrst kret_neg1
backstore_free: jrst kret_neg1
        .bss
        .globl backstore_bitmap
        .globl backstore_blocks
        .globl backstore_blocks_used
        .globl backstore_enabled
backstore_bitmap: .block 1
backstore_blocks: .block 1
backstore_blocks_used: .block 1
backstore_enabled: .block 1
