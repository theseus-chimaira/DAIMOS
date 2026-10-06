/**
 * @file memfs_data_pdp6.s
 * @brief Compact PDP-6 MEMFS mutable-data allocator and backing runtime.
 *
 * The portable policy lives in memfs_data.c.  KCC expands its small free-list
 * walks and eviction loops into large save frames, so the PDP-6 target keeps
 * the same policy here with private register ABIs.  Four 02000-word maximum
 * chunks are suballocated by address-ordered free lists; completely free
 * chunks return to MM, and pressure eviction writes unbacked files to the
 * shared backstore before releasing the whole chunk.
 */

        .text
        .globl  memfs_data_init
        .globl  memfs_data_destroy
        .globl  memfs_data_ensure
        .globl  memfs_data_dirty
        .globl  memfs_data_reclaim
        .globl  memfs_resize

        .globl  mm_alloc
        .globl  mm_free
        .globl  backstore_blocks
        .globl  backstore_alloc
        .globl  backstore_free
        .globl  backstore_read
        .globl  backstore_write
        .globl  fs_copy_words
        .globl  fs_zero_words

        .equ    MEMFS_CHUNKS,4
        .equ    MEMFS_CHUNK_WORDS,02000
        .equ    MEMFS_MM_OWNER,011
        .equ    MEMFS_SECTOR_WORDS,0200
        .equ    MEMFS_PROCESS_RESERVE,0200
        .equ    MM_TYPE_KERNEL_DYNAMIC,3

        ; struct memfs: nodes, node_count, pool, pool_words, used_words, image.
        .equ    MEMFS_NODES,0
        .equ    MEMFS_NODE_COUNT,1
        .equ    MEMFS_POOL,2
        .equ    MEMFS_USED_WORDS,4
        ; struct memfs_node: five name/meta words followed by packed data.
        .equ    MEMFS_NODE_META,5
        .equ    MEMFS_NODE_DATA,6

; Round AC1 logical words to a 128-word sector, with one sector minimum.
; Private leaf helper; AC2..AC7 are preserved.
memfs_alloc_words:
        caige   1,MEMFS_SECTOR_WORDS
        jrst    memfs_alloc_words_min
        addi    1,MEMFS_SECTOR_WORDS-1
        lsh     1,-7
        lsh     1,7
        popj    17,
memfs_alloc_words_min:
        movei   1,MEMFS_SECTOR_WORDS
        popj    17,

; Allocate AC2 words from chunk AC1.  Return AC1=0/-1 and AC2=base on success.
; Free header: word 0 size, word 1 next.  AC3..AC7 are scratch.
memfs_chunk_alloc:
        setz    5,                     ; previous free extent
        move    4,2(1)                 ; current free extent
memfs_chunk_alloc_loop:
        jumpe   4,memfs_chunk_alloc_fail
        move    6,(4)
        camge   6,2
        jrst    memfs_chunk_alloc_next
        came    6,2
        jrst    memfs_chunk_alloc_split
        move    7,1(4)
        jumpe   5,memfs_chunk_alloc_head
        movem   7,1(5)
        jrst    memfs_chunk_alloc_take
memfs_chunk_alloc_head:
        movem   7,2(1)
memfs_chunk_alloc_take:
        move    2,4
        setz    1,
        popj    17,
memfs_chunk_alloc_split:
        sub     6,2
        movem   6,(4)
        add     4,6
        move    2,4
        setz    1,
        popj    17,
memfs_chunk_alloc_next:
        move    5,4
        move    4,1(4)
        jrst    memfs_chunk_alloc_loop
memfs_chunk_alloc_fail:
        seto    1,
        popj    17,

; Allocate AC1 logical words.  Return AC1=0/-1 and AC2=physical base.
; Preserves the normal C callee-saved register set used by public callers.
memfs_data_alloc:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1
        pushj   17,memfs_alloc_words
        move    011,1                  ; rounded need

        movei   012,MEMFS_CHUNKS
        movei   013,memfs_data_chunks
memfs_data_alloc_existing:
        skipn   (013)
        jrst    memfs_data_alloc_existing_next
        move    1,013
        move    2,011
        pushj   17,memfs_chunk_alloc
        jumpe   1,memfs_data_alloc_done
memfs_data_alloc_existing_next:
        addi    013,3
        sojg    012,memfs_data_alloc_existing

        move    014,memfs_data_limit
        sub     014,memfs_data_capacity ; remaining configured capacity
        camge   014,011
        jrst    memfs_data_alloc_fail

        movei   012,MEMFS_CHUNKS
        movei   013,memfs_data_chunks
memfs_data_alloc_find_slot:
        skipn   (013)
        jrst    memfs_data_alloc_have_slot
        addi    013,3
        sojg    012,memfs_data_alloc_find_slot
        jrst    memfs_data_alloc_fail

memfs_data_alloc_have_slot:
        move    010,011
        caige   010,MEMFS_CHUNK_WORDS
        movei   010,MEMFS_CHUNK_WORDS
        camle   010,014
        move    010,014
        ; Clamp a short final chunk to a complete sector.
        lsh     010,-7
        lsh     010,7
        camge   010,011
        jrst    memfs_data_alloc_fail

        push    17,[0]                 ; mm_alloc result word
        movei   1,(17)
        push    17,1                   ; fifth C argument: &base
        setom   memfs_data_allocating
        move    1,010
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        setz    4,                     ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        setzm   memfs_data_allocating
        sub     17,[1,,1]              ; discard fifth argument
        jumpn   1,memfs_data_alloc_mm_fail
        move    2,(17)                 ; allocated chunk base
        sub     17,[1,,1]              ; discard result word

        movem   2,(013)
        movem   010,1(013)
        movem   2,2(013)
        movem   010,(2)
        setzm   1(2)
        addm    010,memfs_data_capacity
        move    1,013
        move    2,011
        pushj   17,memfs_chunk_alloc
        jrst    memfs_data_alloc_done

memfs_data_alloc_mm_fail:
        sub     17,[1,,1]              ; discard result word
memfs_data_alloc_fail:
        seto    1,
memfs_data_alloc_done:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; Free AC1 base / AC2 logical words back to its containing chunk.
; Private helper; callers need no result.
memfs_data_free:
        move    7,1                    ; base
        move    1,2
        pushj   17,memfs_alloc_words
        move    6,1                    ; rounded size
        movei   3,MEMFS_CHUNKS
        movei   4,memfs_data_chunks
memfs_data_free_find:
        skipn   1,(4)
        jrst    memfs_data_free_find_next
        camge   7,1
        jrst    memfs_data_free_find_next
        move    2,7
        add     2,6
        move    5,1
        add     5,1(4)
        camle   2,5
        jrst    memfs_data_free_find_next
        jrst    memfs_data_free_found
memfs_data_free_find_next:
        addi    4,3
        sojg    3,memfs_data_free_find
        popj    17,

memfs_data_free_found:
        ; Insert address-ordered: AC3=prev, AC5=cur.
        setz    3,
        move    5,2(4)
memfs_data_free_scan:
        jumpe   5,memfs_data_free_insert
        caml    5,7
        jrst    memfs_data_free_insert
        move    3,5
        move    5,1(5)
        jrst    memfs_data_free_scan
memfs_data_free_insert:
        movem   6,(7)
        movem   5,1(7)
        jumpe   3,memfs_data_free_new_head
        movem   7,1(3)
        jrst    memfs_data_free_merge_next
memfs_data_free_new_head:
        movem   7,2(4)

memfs_data_free_merge_next:
        move    5,1(7)
        jumpe   5,memfs_data_free_merge_prev
        move    1,7
        add     1,(7)
        came    1,5
        jrst    memfs_data_free_merge_prev
        move    1,(5)
        addm    1,(7)
        move    1,1(5)
        movem   1,1(7)

memfs_data_free_merge_prev:
        jumpe   3,memfs_data_free_release_check
        move    1,3
        add     1,(3)
        came    1,7
        jrst    memfs_data_free_release_check
        move    1,(7)
        addm    1,(3)
        move    1,1(7)
        movem   1,1(3)
        move    7,3                    ; merged header for whole-chunk test

memfs_data_free_release_check:
        move    1,2(4)
        came    1,(4)
        popj    17,
        came    7,(4)
        popj    17,
        move    1,(7)
        came    1,1(4)
        popj    17,
        skipe   1(7)
        popj    17,
        move    6,(4)                  ; chunk base
        move    7,1(4)                 ; chunk words
        setzm   (4)
        setzm   1(4)
        setzm   2(4)
        movn    1,7
        addm    1,memfs_data_capacity
        move    1,6
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        pushj   17,mm_free
        popj    17,

; Drop slot AC1's existing backing allocation, if any.
memfs_backing_drop:
        skipn   3,memfs_data_fs
        popj    17,
        skipn   4,MEMFS_POOL(3)
        popj    17,
        add     4,1
        skipn   2,(4)
        popj    17,
        setzm   (4)
        hlrz    1,2
        hrrz    2,2
        pushj   17,backstore_free
        popj    17,

; void memfs_data_init(struct memfs *fs, kword_t limit)
memfs_data_init:
        setzm   memfs_data_chunks
        movei   3,memfs_data_chunks+1
        hrli    3,memfs_data_chunks
        blt     3,memfs_data_chunks+013 ; 12 chunk words
        movem   1,memfs_data_fs
        movem   2,memfs_data_limit
        setzm   memfs_data_capacity
        setzm   memfs_data_allocating
        popj    17,

; int memfs_data_ensure(struct memfs *fs, unsigned int slot)
memfs_data_ensure:
        add     17,[5,,5]
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)
        move    010,1                  ; fs
        move    011,2                  ; slot
        jumpe   010,memfs_data_ensure_fail
        came    010,memfs_data_fs
        jrst    memfs_data_ensure_fail
        caml    011,MEMFS_NODE_COUNT(010)
        jrst    memfs_data_ensure_fail
        move    012,011
        imuli   012,7
        add     012,MEMFS_NODES(010)   ; node
        hrrz    013,MEMFS_NODE_DATA(012)
        jumpe   013,memfs_data_ensure_ok
        hlrz    1,MEMFS_NODE_DATA(012)
        jumpn   1,memfs_data_ensure_ok
        move    014,MEMFS_POOL(010)
        add     014,011
        move    014,(014)              ; backing first,,blocks
        jumpe   014,memfs_data_ensure_fail
        move    1,013
        pushj   17,memfs_data_alloc
        jumpn   1,memfs_data_ensure_fail
        move    010,2                  ; allocated base; fs no longer needed
        hlrz    1,014
        hrrz    2,014
        move    3,010
        pushj   17,backstore_read
        jumpe   1,memfs_data_ensure_publish
        move    1,010
        move    2,013
        pushj   17,memfs_data_free
        jrst    memfs_data_ensure_fail
memfs_data_ensure_publish:
        hrlz    1,010
        ior     1,013
        movem   1,MEMFS_NODE_DATA(012)
memfs_data_ensure_ok:
        setz    1,
        jrst    memfs_data_ensure_return
memfs_data_ensure_fail:
        seto    1,
memfs_data_ensure_return:
        movei   0,010
        hrli    0,-4(17)
        blt     0,014
        sub     17,[5,,5]
        popj    17,

; void memfs_data_dirty(unsigned int slot)
memfs_data_dirty:
        skipn   3,memfs_data_fs
        popj    17,
        caml    1,MEMFS_NODE_COUNT(3)
        popj    17,
        jrst    memfs_backing_drop

; int memfs_resize(struct memfs *fs, unsigned int slot, unsigned int words)
memfs_resize:
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)
        move    010,1
        move    011,2
        move    012,3
        jumpe   010,memfs_resize_fail
        caml    011,MEMFS_NODE_COUNT(010)
        jrst    memfs_resize_fail
        tlne    012,0777777            ; logical size must fit RH18
        jrst    memfs_resize_fail
        move    013,011
        imuli   013,7
        add     013,MEMFS_NODES(010)
        move    1,MEMFS_NODE_META(013)
        andi    1,6
        caie    1,4                    ; MEMFS_F_WRITABLE exactly
        jrst    memfs_resize_fail
        hrrz    014,MEMFS_NODE_DATA(013)
        jumpe   014,memfs_resize_resident
        move    1,010
        move    2,011
        pushj   17,memfs_data_ensure
        jumpn   1,memfs_resize_fail
memfs_resize_resident:
        hlrz    015,MEMFS_NODE_DATA(013)
        came    012,014
        jrst    memfs_resize_change
        setz    1,
        jrst    memfs_resize_return

memfs_resize_change:
        jumpn   012,memfs_resize_grow
        jumpe   015,memfs_resize_zero_drop
        move    1,015
        move    2,014
        pushj   17,memfs_data_free
memfs_resize_zero_drop:
        move    1,011
        pushj   17,memfs_backing_drop
        setzm   MEMFS_NODE_DATA(013)
        movn    1,014
        addm    1,MEMFS_USED_WORDS(010)
        setz    1,
        jrst    memfs_resize_return

memfs_resize_grow:
        move    1,012
        pushj   17,memfs_data_alloc
        jumpn   1,memfs_resize_fail
        move    016,2                  ; new base
        move    3,014
        camle   3,012
        move    3,012                  ; copy=min(old,new)
        move    1,015
        move    2,016
        pushj   17,fs_copy_words
        move    1,012
        pushj   17,memfs_alloc_words
        sub     1,3                    ; rounded tail after copied words
        move    2,1
        move    1,016
        add     1,3
        pushj   17,fs_zero_words
        jumpe   015,memfs_resize_drop_backing
        move    1,015
        move    2,014
        pushj   17,memfs_data_free
memfs_resize_drop_backing:
        move    1,011
        pushj   17,memfs_backing_drop
        hrlz    1,016
        ior     1,012
        movem   1,MEMFS_NODE_DATA(013)
        move    1,012
        sub     1,014
        addm    1,MEMFS_USED_WORDS(010)
        setz    1,
        jrst    memfs_resize_return
memfs_resize_fail:
        seto    1,
memfs_resize_return:
        movei   0,010
        hrli    0,-6(17)
        blt     0,016
        sub     17,[7,,7]
        popj    17,

; Evict one whole chunk. AC1=chunk pointer, return released words or zero.
memfs_evict_chunk:
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)
        move    010,1
        move    011,memfs_data_fs
        jumpe   011,memfs_evict_zero
        skipn   (010)
        jrst    memfs_evict_zero
        movei   012,1                  ; slot 0 is root metadata

memfs_evict_back_loop:
        caml    012,MEMFS_NODE_COUNT(011)
        jrst    memfs_evict_clear_start
        move    013,012
        imuli   013,7
        add     013,MEMFS_NODES(011)
        hlrz    014,MEMFS_NODE_DATA(013)
        hrrz    015,MEMFS_NODE_DATA(013)
        jumpe   015,memfs_evict_back_next
        camge   014,(010)
        jrst    memfs_evict_back_next
        move    1,(010)
        add     1,1(010)
        caml    014,1
        jrst    memfs_evict_back_next
        move    1,MEMFS_POOL(011)
        add     1,012
        skipe   (1)
        jrst    memfs_evict_back_next

        move    1,015
        pushj   17,memfs_alloc_words
        lsh     1,-7
        move    016,1                  ; backing blocks
        push    17,[0]                 ; allocated first block
        movei   3,(17)
        move    1,016
        movei   2,MEMFS_PROCESS_RESERVE
        pushj   17,backstore_alloc
        jumpn   1,memfs_evict_back_fail
        move    1,(17)
        move    2,016
        move    3,014
        pushj   17,backstore_write
        jumpe   1,memfs_evict_back_store
        move    1,(17)
        move    2,016
        pushj   17,backstore_free
memfs_evict_back_fail:
        sub     17,[1,,1]
        jrst    memfs_evict_zero
memfs_evict_back_store:
        hrlz    1,(17)
        ior     1,016
        move    2,MEMFS_POOL(011)
        add     2,012
        movem   1,(2)
        sub     17,[1,,1]
memfs_evict_back_next:
        aoja    012,memfs_evict_back_loop

memfs_evict_clear_start:
        movei   012,1
memfs_evict_clear_loop:
        caml    012,MEMFS_NODE_COUNT(011)
        jrst    memfs_evict_release
        move    013,012
        imuli   013,7
        add     013,MEMFS_NODES(011)
        hlrz    014,MEMFS_NODE_DATA(013)
        camge   014,(010)
        jrst    memfs_evict_clear_next
        move    1,(010)
        add     1,1(010)
        caml    014,1
        jrst    memfs_evict_clear_next
        hrrzs   MEMFS_NODE_DATA(013)
memfs_evict_clear_next:
        aoja    012,memfs_evict_clear_loop

memfs_evict_release:
        move    014,(010)
        move    015,1(010)
        move    1,014
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        pushj   17,mm_free
        jumpn   1,memfs_evict_zero
        setzm   (010)
        setzm   1(010)
        setzm   2(010)
        movn    1,015
        addm    1,memfs_data_capacity
        move    1,015
        jrst    memfs_evict_return
memfs_evict_zero:
        setz    1,
memfs_evict_return:
        movei   0,010
        hrli    0,-6(17)
        blt     0,016
        sub     17,[7,,7]
        popj    17,

; kword_t memfs_data_reclaim(kword_t wanted)
memfs_data_reclaim:
        push    17,010
        push    17,011
        skipe   memfs_data_allocating
        jrst    memfs_data_reclaim_zero
        skipn   backstore_blocks
        jrst    memfs_data_reclaim_zero
        move    010,1                  ; wanted
        setz    011,                   ; released
        movei   2,MEMFS_CHUNKS
        movei   3,memfs_data_chunks
memfs_data_reclaim_loop:
        caml    011,010
        jrst    memfs_data_reclaim_done
        push    17,2
        push    17,3
        move    1,3
        pushj   17,memfs_evict_chunk
        add     011,1
        pop     17,3
        pop     17,2
        addi    3,3
        sojg    2,memfs_data_reclaim_loop
memfs_data_reclaim_done:
        move    1,011
        jrst    memfs_data_reclaim_return
memfs_data_reclaim_zero:
        setz    1,
memfs_data_reclaim_return:
        pop     17,011
        pop     17,010
        popj    17,

; void memfs_data_destroy(void)
memfs_data_destroy:
        push    17,010
        push    17,011
        skipn   010,memfs_data_fs
        jrst    memfs_data_destroy_chunks
        setz    011,
memfs_data_destroy_backing:
        caml    011,MEMFS_NODE_COUNT(010)
        jrst    memfs_data_destroy_chunks
        move    1,011
        pushj   17,memfs_backing_drop
        aoja    011,memfs_data_destroy_backing

memfs_data_destroy_chunks:
        movei   011,MEMFS_CHUNKS
        movei   010,memfs_data_chunks
memfs_data_destroy_chunk_loop:
        skipn   1,(010)
        jrst    memfs_data_destroy_chunk_next
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        pushj   17,mm_free
memfs_data_destroy_chunk_next:
        addi    010,3
        sojg    011,memfs_data_destroy_chunk_loop
        setz    1,
        setz    2,
        pushj   17,memfs_data_init
        pop     17,011
        pop     17,010
        popj    17,

        .bss
; Four three-word chunk descriptors plus singleton allocator state.
memfs_data_chunks:
        .block  014
memfs_data_fs:
        .block  1
memfs_data_limit:
        .block  1
memfs_data_capacity:
        .block  1
memfs_data_allocating:
        .block  1

