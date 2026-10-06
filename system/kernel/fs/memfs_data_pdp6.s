/**
 * @file memfs_data_pdp6.s
 * @brief Compact PDP-6 MEMFS mutable-data allocator and backing runtime.
 *
 * The portable policy lives in memfs_data.c.  KCC expands its small free-list
 * walks and eviction loops into large save frames, so the PDP-6 target keeps
 * the same policy here with private register ABIs.  Ordinary 02000-word chunks
 * contain only eight 0200-word allocation sectors, so one packed state,,base
 * word per chunk replaces the former three-word descriptor plus in-chunk
 * linked free-list headers.  Allocations larger than 02000 words are already
 * dedicated whole chunks and retain that behavior.  Completely free chunks
 * return to MM, and pressure eviction writes unbacked files to the shared
 * backstore before releasing the whole chunk.
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
        .globl  memfs_mres_fs

        .equ    MEMFS_CHUNKS,4
        .equ    MEMFS_CHUNK_WORDS,02000
        .equ    MEMFS_MM_OWNER,011
        .equ    MEMFS_SECTOR_WORDS,0200
        .equ    MEMFS_PROCESS_RESERVE,0200
        .equ    MM_TYPE_KERNEL_DYNAMIC,3
        .equ    MEMFS_CHUNK_LARGE,0400000
        .equ    MEMFS_CHUNK_BITMAP,0377
        .equ    MEMFS_CHUNK_COUNT_SHIFT,010

        ; struct memfs: nodes, pool, pool_words, used_words.
        .equ    MEMFS_NODES,0
        .equ    MEMFS_POOL,1
        .equ    MEMFS_USED_WORDS,3
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

; Decode descriptor AC1. Return AC1=base, AC2=chunk words, AC3=packed state.
; Ordinary state is sector-count<<8 | allocation bitmap; a large dedicated
; chunk uses MEMFS_CHUNK_LARGE | sector-count.
memfs_chunk_bounds:
        hlrz    3,(1)
        hrrz    1,(1)
        move    2,3
        trne    3,MEMFS_CHUNK_LARGE
        jrst    memfs_chunk_bounds_large
        lsh     2,-MEMFS_CHUNK_COUNT_SHIFT
        andi    2,017
        jrst    memfs_chunk_bounds_words
memfs_chunk_bounds_large:
        andi    2,0377777
memfs_chunk_bounds_words:
        lsh     2,7
        popj    17,

; Allocate AC2 sectors from ordinary chunk descriptor AC1.
; Return AC1=0/-1 and AC2=physical base on success. AC3..AC7 are scratch.
memfs_chunk_alloc:
        hlrz    3,(1)
        trne    3,MEMFS_CHUNK_LARGE
        jrst    memfs_chunk_alloc_fail
        move    4,3
        lsh     4,-MEMFS_CHUNK_COUNT_SHIFT
        andi    4,017
        camle   2,4
        jrst    memfs_chunk_alloc_fail
        movei   5,1
        lsh     5,0(2)
        subi    5,1
        setz    6,
memfs_chunk_alloc_loop:
        move    7,3
        andi    7,MEMFS_CHUNK_BITMAP
        and     7,5
        jumpe   7,memfs_chunk_alloc_take
        lsh     5,1
        aoj     6,
        move    7,6
        add     7,2
        camle   7,4
        jrst    memfs_chunk_alloc_fail
        jrst    memfs_chunk_alloc_loop
memfs_chunk_alloc_take:
        ior     3,5
        hrlm    3,(1)
        hrrz    7,(1)
        move    2,6
        lsh     2,7
        add     2,7
        setz    1,
        popj    17,
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
        pushj   17,memfs_alloc_words
        move    011,1                  ; rounded need words
        move    014,1
        lsh     014,-7                 ; requested sectors
        setz    010,                   ; current allocated chunk capacity

        movei   012,MEMFS_CHUNKS
        movei   013,memfs_data_chunks
memfs_data_alloc_existing:
        skipn   (013)
        jrst    memfs_data_alloc_existing_next
        move    1,013
        pushj   17,memfs_chunk_bounds
        add     010,2
        move    1,013
        move    2,014
        pushj   17,memfs_chunk_alloc
        jumpe   1,memfs_data_alloc_done
memfs_data_alloc_existing_next:
        addi    013,1
        sojg    012,memfs_data_alloc_existing

        move    1,memfs_mres_fs+2      ; configured mutable-data ceiling
        sub     1,010
        move    010,1                  ; remaining configured capacity
        camge   010,011
        jrst    memfs_data_alloc_fail

        movei   012,MEMFS_CHUNKS
        movei   013,memfs_data_chunks
memfs_data_alloc_find_slot:
        skipn   (013)
        jrst    memfs_data_alloc_have_slot
        addi    013,1
        sojg    012,memfs_data_alloc_find_slot
        jrst    memfs_data_alloc_fail

memfs_data_alloc_have_slot:
        move    012,011
        caige   012,MEMFS_CHUNK_WORDS
        movei   012,MEMFS_CHUNK_WORDS
        camle   012,010
        move    012,010
        ; Clamp a short final chunk to a complete sector.
        lsh     012,-7
        lsh     012,7
        camge   012,011
        jrst    memfs_data_alloc_fail

        push    17,[0]                 ; mm_alloc result word
        movei   1,(17)
        push    17,1                   ; fifth C argument: &base
        setom   memfs_data_allocating
        move    1,012
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        setz    4,                     ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        setzm   memfs_data_allocating
        sub     17,[1,,1]              ; discard fifth argument
        jumpn   1,memfs_data_alloc_mm_fail
        move    2,(17)                 ; allocated chunk base
        sub     17,[1,,1]              ; discard result word

        move    4,012
        lsh     4,-7                    ; allocated chunk sectors
        caile   011,MEMFS_CHUNK_WORDS
        jrst    memfs_data_alloc_large
        lsh     4,MEMFS_CHUNK_COUNT_SHIFT
        move    5,2
        hrl     5,4
        movem   5,(013)
        move    1,013
        move    2,014
        pushj   17,memfs_chunk_alloc
        jrst    memfs_data_alloc_done
memfs_data_alloc_large:
        iori    4,MEMFS_CHUNK_LARGE
        move    5,2
        hrl     5,4
        movem   5,(013)
        setz    1,                     ; dedicated chunk itself is the extent
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

; Free AC1 base / AC2 logical words back to its containing chunk.  Large
; chunks are dedicated allocations; ordinary chunks clear sector bitmap bits.
; A completely free chunk returns immediately to MM.
memfs_data_free:
        move    7,1                    ; base
        move    1,2
        pushj   17,memfs_alloc_words
        move    6,1                    ; rounded words
        movei   0,MEMFS_CHUNKS
        movei   4,memfs_data_chunks
memfs_data_free_find:
        skipn   (4)
        jrst    memfs_data_free_find_next
        move    1,4
        pushj   17,memfs_chunk_bounds  ; base, words, state
        camge   7,1
        jrst    memfs_data_free_find_next
        move    5,7
        add     5,6
        add     2,1
        camle   5,2
        jrst    memfs_data_free_find_next
        jrst    memfs_data_free_found
memfs_data_free_find_next:
        addi    4,1
        sojg    0,memfs_data_free_find
        popj    17,

memfs_data_free_found:
        trne    3,MEMFS_CHUNK_LARGE
        jrst    memfs_data_free_release
        move    0,6
        lsh     0,-7                   ; sectors to clear
        sub     7,1                    ; sector position within chunk
        lsh     7,-7
        movei   5,1
        lsh     5,0(0)
        subi    5,1                    ; low NEED-sector mask
        lsh     5,0(7)
        andi    3,MEMFS_CHUNK_BITMAP
        andca   3,5                    ; bitmap &= ~released mask
        jumpe   3,memfs_data_free_release
        hlrz    5,(4)
        andi    5,0777400              ; retain sector-count field
        ior     5,3
        hrlm    5,(4)
        popj    17,

memfs_data_free_release:
        hrrz    6,(4)                  ; chunk base
        move    1,4
        pushj   17,memfs_chunk_bounds
        move    7,2                    ; chunk words
        setzm   (4)
        move    1,6
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        pushj   17,mm_free
        popj    17,

; Drop slot AC1's existing backing allocation, if any.
memfs_backing_drop:
        movei   3,memfs_mres_fs
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
        blt     3,memfs_data_chunks+3   ; four packed chunk descriptors
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
        caile   011,077
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
        movei   3,memfs_mres_fs
        caile   1,077
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
        caile   011,077
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
        movei   011,memfs_mres_fs
        skipn   MEMFS_NODES(011)
        jrst    memfs_evict_zero
        skipn   (010)
        jrst    memfs_evict_zero
        move    1,010
        pushj   17,memfs_chunk_bounds
        move    014,1                  ; chunk base
        move    015,2                  ; chunk words
        movei   012,1                  ; slot 0 is root metadata

memfs_evict_back_loop:
        caile   012,077
        jrst    memfs_evict_clear_start
        move    013,012
        imuli   013,7
        add     013,MEMFS_NODES(011)
        hlrz    5,MEMFS_NODE_DATA(013)
        hrrz    6,MEMFS_NODE_DATA(013)
        jumpe   6,memfs_evict_back_next
        camge   5,014
        jrst    memfs_evict_back_next
        move    1,014
        add     1,015
        caml    5,1
        jrst    memfs_evict_back_next
        move    1,MEMFS_POOL(011)
        add     1,012
        skipe   (1)
        jrst    memfs_evict_back_next

        move    1,6
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
        hlrz    3,MEMFS_NODE_DATA(013) ; backstore_alloc may clobber AC5
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
        caile   012,077
        jrst    memfs_evict_release
        move    013,012
        imuli   013,7
        add     013,MEMFS_NODES(011)
        hlrz    5,MEMFS_NODE_DATA(013)
        camge   5,014
        jrst    memfs_evict_clear_next
        move    1,014
        add     1,015
        caml    5,1
        jrst    memfs_evict_clear_next
        hrrzs   MEMFS_NODE_DATA(013)
memfs_evict_clear_next:
        aoja    012,memfs_evict_clear_loop

memfs_evict_release:
        move    1,014
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        pushj   17,mm_free
        jumpn   1,memfs_evict_zero
        setzm   (010)
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
        addi    3,1
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
        movei   010,memfs_mres_fs
        skipn   MEMFS_NODES(010)
        jrst    memfs_data_destroy_chunks
        setz    011,
memfs_data_destroy_backing:
        caile   011,077
        jrst    memfs_data_destroy_chunks
        move    1,011
        pushj   17,memfs_backing_drop
        aoja    011,memfs_data_destroy_backing

memfs_data_destroy_chunks:
        movei   011,MEMFS_CHUNKS
        movei   010,memfs_data_chunks
memfs_data_destroy_chunk_loop:
        skipn   (010)
        jrst    memfs_data_destroy_chunk_next
        hrrz    1,(010)
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,MEMFS_MM_OWNER
        pushj   17,mm_free
memfs_data_destroy_chunk_next:
        addi    010,1
        sojg    011,memfs_data_destroy_chunk_loop
        setz    1,
        setz    2,
        pushj   17,memfs_data_init
        pop     17,011
        pop     17,010
        popj    17,

        .bss
; Four packed state,,base chunk descriptors plus singleton allocator state.
memfs_data_chunks:
        .block  4
memfs_data_allocating:
        .block  1

