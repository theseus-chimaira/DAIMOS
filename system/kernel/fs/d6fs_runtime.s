; d6fs_runtime.s -- compact resident D6FS packed-format primitives.
;
; These routines replace C shift/divide machinery on the PDP-6/PDP-10 hot
; paths.  They implement the on-disk D6FS V2 bit layout directly; policy and
; crash-ordering remain in C.
        .text
        .globl  kret_zero
        .globl  kret_one
        .globl  kret_neg1

; FCB decode/validation is implemented in d6fs.c.

        .globl  d6fs_dirent_decode_valid
; int d6fs_dirent_decode_valid(ent, fcb_count, info)
d6fs_dirent_decode_valid:
        jumpe   1,kret_zero
        jumpe   2,kret_zero
        jumpe   3,kret_zero
        move    4,5(1)
        trne    4,0777777                ; low half must be zero
        jrst    kret_zero
        move    5,3
        hrl     5,1
        blt     5,3(3)                   ; copy four packed name words
        move    5,4(1)
        ldb     6,[POINT 24,5,23]         ; hash24
        movem   6,4(3)
        ldb     6,[POINT 3,5,26]          ; type bits 9..11
        movem   6,5(3)
        move    7,5
        andi    7,0777
        movem   7,6(3)
        hlrz    4,4
        movem   4,7(3)                  ; child_fcb
        jumpn   4,d6fs_dirent_used
        move    6,(1)
        ior     6,1(1)
        ior     6,2(1)
        ior     6,3(1)
        ior     6,5                      ; raw word 4 must also be zero
        jumpe   6,d6fs_dirent_valid
        jrst    kret_zero

d6fs_dirent_used:
        caml    4,2                      ; child_fcb >= fcb_count
        jrst    kret_zero
        jumpe   6,kret_zero    ; FREE type forbidden
        caile   6,4
        jrst    kret_zero
        move    4,(1)
        ior     4,1(1)
        ior     4,2(1)
        ior     4,3(1)
        jumpe   4,kret_zero

d6fs_dirent_valid:
        jrst    kret_one

        .globl  d6fs_extent_decode
; int d6fs_extent_decode(run, high, startp, blocksp)
d6fs_extent_decode:
        jumpe   3,kret_neg1
        jumpe   4,kret_neg1
        tdne    2,[-040]                 ; high must fit five bits
        jrst    kret_neg1
        ldb     5,[POINT 24,1,23]
        movem   5,(3)
        lsh     2,014
        andi    1,07777
        ior     2,1
        addi    2,1
        movem   2,(4)
        jrst    kret_zero

        .globl  d6fs_extent_high_get
; unsigned int d6fs_extent_high_get(word, extent)
d6fs_extent_high_get:
        caile   2,6
        jrst    kret_zero
        move    3,2
        lsh     3,2
        add     3,2                      ; shift = extent * 5
        movn    3,3
        lsh     1,0(3)
        andi    1,037
        popj    17,

; kword_t d6fs_file_block(fcb, file_block)
;
; Return the logical block directly; D6FS_CACHE_INVALID (-1) means that the
; requested file block is outside all extents.  Logical D6FS block numbers are
; only 24 bits, so the sentinel cannot collide with valid media.
        .globl  d6fs_file_block
d6fs_file_block:
        jumpe   1,kret_neg1
        ldb     3,[POINT 4,(1),31]       ; extent_count
        jumpe   3,kret_neg1
        move    4,5(1)                   ; packed length-high fields
        movei   5,6(1)                   ; current extent run

d6fs_file_block_loop:
        move    6,(5)
        move    7,6
        andi    7,07777                  ; low 12 length bits
        move    0,4
        andi    0,037                    ; current high five bits
        lsh     0,014
        ior     7,0
        addi    7,1                      ; blocks in this extent
        caml    2,7
        jrst    d6fs_file_block_next
        ldb     1,[POINT 24,6,23]         ; 24-bit start
        add     1,2
        popj    17,

d6fs_file_block_next:
        sub     2,7
        lsh     4,-5                     ; next length-high field
        addi    5,1
        sojg    3,d6fs_file_block_loop
        jrst    kret_neg1                ; file block lies past final extent

        .globl  d6fs_name_hash24
; kword_t d6fs_name_hash24(words, chars)
;
; Hash packed SIXBIT directly with ILDB; this avoids C division/modulo and
; per-character word shifting.  SIXBIT bytes are converted to ASCII by +040.
d6fs_name_hash24:
        jumpe   1,kret_zero
        jumpe   2,kret_zero
        caile   2,030                    ; maximum 24 characters
        jrst    kret_zero
        move    3,[POINT 6,0]
        hrr     3,1
        setz    4,                       ; h

d6fs_hash_loop:
        ildb    5,3
        addi    5,040
        move    6,4
        lsh     6,5
        move    7,4
        lsh     7,-023                   ; h >> 19 decimal
        xor     6,7
        xor     6,5
        and     6,[077777777]
        move    4,6
        sojg    2,d6fs_hash_loop
        move    1,4
        popj    17,


        .globl  fs_block_workspace
        .globl  bcache_fetch
        .globl  bcache_store
        .globl  bcache_reclaim
        .globl  d6fs_reader_get_block
; const kword_t *d6fs_reader_get_block(reader, logical)
;
; A successful load always exposes reader->cache, so return that address
; directly instead of forcing every caller to allocate a pointer temporary.
d6fs_reader_get_block:
        jumpe   1,kret_zero
        caml    2,6(1)                   ; logical >= total_blocks
        jrst    kret_zero
        ; fs_block_workspace is shared by all filesystem providers.  The
        ; reader-local tag therefore cannot prove that the workspace still
        ; contains this D6FS block: DTFS/TSFS/MEMFS or boot code may have
        ; reused it since the last D6FS access.  Always validate/copy through
        ; the shared BCACHE tag instead of trusting the stale local tag.
        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    011,2
        move    012,1(010)               ; low six bits are VFS mount id
        andi    012,077
        iori    012,01000                ; BCACHE_SOURCE_D6FS
        lsh     012,030                  ; SOURCE12,,BLOCK24
        ior     012,011
        move    1,012
        movei   2,fs_block_workspace
        pushj   17,bcache_fetch
        jumpn   1,d6fs_get_block_cache_hit
        movei   1,016(010)               ; embedded struct fs_backing
        move    2,011
        movei   3,fs_block_workspace     ; shared transfer block
        pushj   17,fs_backing_read
        jumpn   1,d6fs_get_block_read_fail
        move    1,012
        movei   2,fs_block_workspace
        pushj   17,bcache_store          ; miss becomes a clean cache entry
        movei   1,fs_block_workspace
        jrst    d6fs_get_block_read_done
d6fs_get_block_cache_hit:
        movei   1,fs_block_workspace
        jrst    d6fs_get_block_read_done
d6fs_get_block_read_fail:
        setz    1,
d6fs_get_block_read_done:
        jrst    d6fs_restore3

        .globl  d6fs_reader_fcb
; int d6fs_reader_fcb(reader, index, fcb, info)
;
; FCBs are fixed 16-word records and 128-word blocks therefore contain exactly
; eight aligned FCBs.  Use shifts/masks and BLT instead of compiler division
; and a sixteen-iteration copy loop.
d6fs_reader_fcb:
        jumpe   1,kret_neg1
        jumpe   4,kret_neg1
        caml    2,011(1)                 ; index >= fcb_count
        jrst    kret_neg1
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                    ; reader
        move    011,3                    ; destination FCB
        move    012,4                    ; decoded info
        move    013,2                    ; index
        move    5,2
        lsh     5,-3                     ; index / 8
        add     5,010(010)               ; fcb_start + block index
        move    2,5
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_reader_fcb_fail
        move    6,013
        andi    6,7
        lsh     6,4                      ; (index % 8) * 16
        add     1,6                      ; source FCB in cache
        jumpe   011,d6fs_reader_fcb_decode_cache
        move    5,011                    ; BLT source,,destination
        hrl     5,1
        blt     5,017(011)
        move    1,011
d6fs_reader_fcb_decode_cache:
        move    2,6(010)                 ; total_blocks
        move    3,011(010)               ; fcb_count
        move    4,012
        pushj   17,d6fs_fcb_decode_valid
        jumpe   1,d6fs_reader_fcb_fail
        setz    1,
        jrst    d6fs_reader_fcb_done
d6fs_reader_fcb_fail:
        seto    1,
d6fs_reader_fcb_done:
        jrst    d6fs_restore4

        .globl  d6fs_reader_put_fcb
; int d6fs_reader_put_fcb(reader, index, fcb)
d6fs_reader_put_fcb:
        jumpe   1,kret_neg1
        jumpe   3,kret_neg1
        caml    2,011(1)
        jrst    kret_neg1
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                    ; reader
        move    011,3                    ; source FCB
        move    012,2                    ; index
        move    013,2
        lsh     013,-3
        add     013,010(010)             ; logical FCB block
        move    1,010
        move    2,013
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_reader_put_fcb_fail
        move    6,012
        andi    6,7
        lsh     6,4
        add     1,6                      ; destination in shared cache
        move    5,1
        hrl     5,011                    ; source,,destination
        blt     5,017(1)
        move    1,010
        move    2,013
        pushj   17,d6fs_reader_commit_cache
        jrst    d6fs_reader_put_fcb_done
d6fs_reader_put_fcb_fail:
        seto    1,
d6fs_reader_put_fcb_done:
        jrst    d6fs_restore4

; Return quotient map-block in AC1, word index in AC2, bit index in AC3.
; Input index is in AC1.  D6FS map blocks contain 128*36 = 011000 bits.
d6fs_bitmap_pos:
        move    4,1
        idivi   4,011000
        move    1,4
        move    4,5
        idivi   4,044
        move    2,4
        move    3,5
        popj    17,

; Return the MSB-first 36-bit bit mask for bit index AC1 (0..35).
d6fs_bitmap_mask:
        movei   4,043
        sub     4,1
        movei   1,1
        lsh     1,0(4)
        popj    17,

        .globl  d6fs_freemap_state
; int d6fs_freemap_state(reader, logical)
; Return 0 free, 1 allocated, -1 on I/O/range error.
d6fs_freemap_state:
        jumpe   1,kret_neg1
        caml    2,6(1)
        jrst    kret_neg1
        push    17,010
        push    17,011
        push    17,012
        move    010,1                     ; reader
        move    1,2
        pushj   17,d6fs_bitmap_pos
        caml    1,013(010)                ; map block >= freemap_blocks
        jrst    d6fs_freemap_state_fail
        move    011,2                     ; word index
        move    012,3                     ; bit index
        add     1,012(010)                ; freemap_start + mbi
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_freemap_state_fail
        add     1,011
        move    2,(1)
        move    1,012
        pushj   17,d6fs_bitmap_mask
        move    3,1                      ; preserve mask while normalizing result
        setz    1,
        tdne    2,3                      ; clear bit skips the one result
        movei   1,1
        jrst    d6fs_freemap_state_done
d6fs_freemap_state_fail:
        seto    1,
d6fs_freemap_state_done:
        jrst    d6fs_restore3

        .globl  d6fs_free_run
; int d6fs_free_run(reader, start, blocks)
; Clear one contiguous allocation run.  Keep this with the bitmap primitives so
; callers do not pay a compiler-generated loop around d6fs_freemap_set().
d6fs_free_run:
        jumpe   1,kret_neg1
        jumpe   3,kret_neg1
        caml    2,6(1)                   ; start >= total_blocks
        jrst    kret_neg1
        move    4,6(1)
        sub     4,2                      ; blocks available from start
        camle   3,4
        jrst    kret_neg1
        push    17,010
        push    17,011
        push    17,012
        move    010,1                    ; reader
        move    011,2                    ; current logical block
        move    012,3                    ; blocks remaining
d6fs_free_run_loop:
        move    1,010
        move    2,011
        setz    3,                       ; mark free
        pushj   17,d6fs_freemap_set
        jumpn   1,d6fs_free_run_fail
        aoj     011,
        sojg    012,d6fs_free_run_loop
        setz    1,
        jrst    d6fs_free_run_done
d6fs_free_run_fail:
        seto    1,
d6fs_free_run_done:
        jrst    d6fs_restore3

        .globl  d6fs_provider_alloc_run
; int d6fs_provider_alloc_run(max_blocks, startp, blocksp)
;
; Selection only: this routine never changes the free map.  It is provider
; private: the provider call operates on the currently selected mount reader.
; STARTP/BLOCKSP are scratch outputs while scanning and are undefined on error.
d6fs_provider_alloc_run:
        jumpe   1,kret_neg1
        jumpe   2,kret_neg1
        jumpe   3,kret_neg1
        add     17,[5,,5]
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)
        move    011,1                    ; max_blocks
        move    012,2                    ; startp
        move    013,3                    ; blocksp
        move    4,d6fs_active_reader   ; active reader
        move    2,(4)                    ; normalized alloc_cursor
        caml    2,6(4)
        jrst    d6fs_provider_alloc_run_fail
        setz    010,                     ; scanned
        setzm   (013)                    ; running count / blocksp

d6fs_provider_alloc_run_loop:
        move    4,d6fs_active_reader
        caml    010,6(4)
        jrst    d6fs_provider_alloc_run_end
        move    2,(4)
        add     2,010                    ; logical = cursor + scanned
        caml    2,6(4)
        sub     2,6(4)                   ; one wrap is sufficient
        move    014,2                    ; preserve current across helper
        move    1,d6fs_active_reader
        pushj   17,d6fs_freemap_state
        jumpl   1,d6fs_provider_alloc_run_fail
        jumpn   1,d6fs_provider_alloc_run_used
        move    2,(013)                  ; running count
        jumpn   2,d6fs_provider_alloc_run_continue
        movem   014,(012)                ; first free logical block
        jrst    d6fs_provider_alloc_run_add
d6fs_provider_alloc_run_continue:
        move    3,(012)
        add     3,2
        came    014,3                    ; current must be contiguous
        jrst    d6fs_provider_alloc_run_restart
d6fs_provider_alloc_run_add:
        aos     (013)
        move    2,(013)
        caml    2,011
        jrst    d6fs_provider_alloc_run_success
        aoja    010,d6fs_provider_alloc_run_loop
d6fs_provider_alloc_run_restart:
        movem   014,(012)
        movei   2,1
        movem   2,(013)
        caml    2,011
        jrst    d6fs_provider_alloc_run_success
        aoja    010,d6fs_provider_alloc_run_loop
d6fs_provider_alloc_run_used:
        skipn   (013)
        aoja    010,d6fs_provider_alloc_run_loop
        jrst    d6fs_provider_alloc_run_success
d6fs_provider_alloc_run_end:
        skipn   (013)
        jrst    d6fs_provider_alloc_run_fail
d6fs_provider_alloc_run_success:
        setz    1,
        jrst    d6fs_provider_alloc_run_done
d6fs_provider_alloc_run_fail:
        seto    1,
d6fs_provider_alloc_run_done:
        jrst    d6fs_restore5

; Internal: return 1 if map block has a free valid bit, 0 if full, -1 error.
; Scan whole 36-bit words instead of testing as many as 4608 individual bits.
d6fs_map_block_has_free_i:
        jumpe   1,kret_neg1
        caml    2,013(1)
        jrst    kret_neg1
        move    010,1
        move    011,2
        move    3,2
        imuli   3,011000                  ; first logical block represented
        move    012,6(1)
        sub     012,3                     ; valid bits remaining
        camle   012,[011000]
        movei   012,011000
        add     2,012(1)
        pushj   17,d6fs_reader_get_block
        jumpe   1,kret_neg1
        move    2,012
        idivi   2,044                     ; AC2 full words, AC3 remainder
        move    4,1                       ; current bitmap word
        move    5,2
        jumpe   5,d6fs_map_scan_partial

d6fs_map_scan_words:
        move    6,(4)
        came    6,[-1]
        jrst    kret_one
        addi    4,1
        sojg    5,d6fs_map_scan_words

d6fs_map_scan_partial:
        jumpe   3,kret_zero
        movei   5,044
        sub     5,3                       ; 36 - remainder
        seto    6,
        lsh     6,0(5)                    ; mask valid MSB-first bits
        move    7,(4)
        and     7,6
        camn    7,6
        jrst    kret_zero
        jrst    kret_one

; Internal summary bit setter: reader AC1, map index AC2, boolean AC3.
d6fs_summary_set_i:
        ; Internal tail helper for d6fs_freemap_set.  The caller has already
        ; saved AC10-AC14 and does not use their working values afterwards.
        move    010,1
        move    014,3
        move    1,2
        pushj   17,d6fs_bitmap_pos
        jumpn   1,kret_neg1          ; V2 summary always fits one block
        move    011,1(010)                 ; packed runtime state
        lsh     011,-014                   ; summary_start in upper 24 bits
        move    012,2                      ; word index
        move    013,3                      ; bit index
        move    1,011
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,kret_neg1
        add     1,012
        move    012,1                     ; address of bitmap word
        move    1,013
        pushj   17,d6fs_bitmap_mask
        jumpe   014,d6fs_summary_clear
        iorm    1,(012)
        jrst    d6fs_summary_commit
d6fs_summary_clear:
        andcam  1,(012)
d6fs_summary_commit:
        move    2,011
        move    1,010
        jrst    d6fs_reader_commit_cache

        .globl  d6fs_freemap_set
; int d6fs_freemap_set(reader, logical, allocated)
d6fs_freemap_set:
        jumpe   1,kret_neg1
        caml    2,6(1)
        jrst    kret_neg1
        add     17,[5,,5]
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)
        move    010,1
        move    011,2
        move    014,3
        move    1,2
        pushj   17,d6fs_bitmap_pos
        caml    1,013(010)
        jrst    d6fs_freemap_set_fail
        move    011,1                     ; map block index
        move    012,2                     ; word index
        move    013,3                     ; bit index
        add     1,012(010)
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_freemap_set_fail
        add     1,012
        move    012,1
        move    1,013
        pushj   17,d6fs_bitmap_mask
        jumpe   014,d6fs_freemap_clear
        iorm    1,(012)
        jrst    d6fs_freemap_commit
d6fs_freemap_clear:
        andcam  1,(012)
d6fs_freemap_commit:
        move    2,011
        add     2,012(010)
        move    1,010
        pushj   17,d6fs_reader_commit_cache
        jumpn   1,d6fs_freemap_set_fail
        movei   1,1
        jumpe   014,d6fs_freemap_have_summary
        move    1,010
        move    2,011
        pushj   17,d6fs_map_block_has_free_i
        jumpl   1,d6fs_freemap_set_fail
d6fs_freemap_have_summary:
        move    3,1
        move    1,010
        move    2,011
        pushj   17,d6fs_summary_set_i
        jrst    d6fs_freemap_set_done
d6fs_freemap_set_fail:
        seto    1,
d6fs_freemap_set_done:
        jrst    d6fs_restore5


        .globl  fs_copy_words
        .globl  fs_backing_read
        .globl  fs_backing_write

        .globl  fs_zero_block_workspace
        .globl  fs_mres_vector_dispatch
        .globl  d6fs_mres_dispatch
        .globl  d6fs_provider_lookup
        .globl  d6fs_provider_readdir
        .globl  d6fs_provider_stat
        .globl  d6fs_provider_parent
        .globl  d6fs_provider_parent_name
        .globl  d6fs_provider_create_object
        .globl  d6fs_provider_unlink
        .globl  d6fs_provider_rename
        .globl  d6fs_provider_truncate
        .globl  d6fs_provider_chmod
        .globl  d6fs_provider_read_words
        .globl  d6fs_provider_write_words
        .globl  d6fs_provider_sync
        .globl  d6fs_provider_prepare_unmount

; D6FS provider dispatch.  CREATE, MKDIR and SYMLINK share one compact
; object creator.  The wrappers only reshape the generic request ABI.
d6fs_mres_dispatch:
d6fs_mres_reg_dispatch:
        cain    6,022                    ; FS_MRES_OP_MOUNT_UNIT (18)
        jrst    d6fs_mount_validated
        ldb     7,[POINT 6,1,11]        ; vnode mount id 1..4
        sojl    7,kret_neg1        ; convert to zero-based slot
        caile   7,3
        jrst    kret_neg1
        move    7,d6fs_reader_slots(7)
        jumpe   7,kret_neg1
        movem   7,d6fs_active_reader
        cain    6,024                    ; FS_MRES_OP_D6FS_REMOUNT (20)
        jrst    d6fs_provider_toggle_state
        cain    6,025                    ; FS_MRES_OP_SPACE (21)
        jrst    d6fs_provider_space
        move    7,[d6fs_mres_vector]
        jrst    fs_mres_vector_dispatch

; Return exact filesystem capacity and allocated words.  Scan the freemap a
; word at a time; counting set bits with x &= x-1 makes cost proportional to
; map words plus allocated blocks rather than to total filesystem blocks.
d6fs_provider_space:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,d6fs_active_reader
        move    011,6(010)              ; valid bits remaining
        setz    012,                    ; map block index
        setz    013,                    ; allocated blocks
d6fs_provider_space_map:
        jumpe   011,d6fs_provider_space_done
        caml    012,013(010)            ; freemap_blocks
        jrst    d6fs_provider_space_fail
        move    2,012(010)              ; freemap_start
        add     2,012
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_provider_space_fail
        move    4,1                     ; current map word
        move    5,011                   ; valid bits in this map block
        camle   5,[011000]
        movei   5,011000
        move    6,5
        idivi   6,044                   ; AC6 full words, AC7 remainder
d6fs_provider_space_words:
        jumpe   6,d6fs_provider_space_partial
        move    1,(4)
d6fs_provider_space_bits:
        jumpe   1,d6fs_provider_space_word_done
        move    2,1
        subi    2,1
        and     1,2
        aoj     013,
        jrst    d6fs_provider_space_bits
d6fs_provider_space_word_done:
        aoj     4,
        sojg    6,d6fs_provider_space_words
        jumpe   7,d6fs_provider_space_next_map
d6fs_provider_space_partial:
        movei   2,044
        sub     2,7                     ; 36 - valid tail bits
        seto    3,
        lsh     3,0(2)                  ; valid bits are MSB-first
        move    1,(4)
        and     1,3
        setz    7,                      ; tail consumed on this pass
        movei   6,1                    ; reuse common popcount completion
        jrst    d6fs_provider_space_bits
d6fs_provider_space_next_map:
        sub     011,5
        aoja    012,d6fs_provider_space_map
d6fs_provider_space_done:
        move    1,6(010)
        lsh     1,7                     ; 128 words/block
        move    2,013
        lsh     2,7
        jrst    d6fs_restore4
d6fs_provider_space_fail:
        seto    1,
        jrst    d6fs_restore4



        .globl  vfs_mount_prevalidated
        .globl  mm_alloc
        .globl  mm_free
; Provider-private runtime mount entry.
; AC1=validated 17-word handoff, AC2=target vnode, AC3=flags.
; Secondary mounts are read-only until the later remount/recovery phase defines
; the complete writable transition protocol.
d6fs_mount_validated:
        jumpe   1,kret_neg1
        caie    3,1                      ; VFS_MOUNT_RDONLY
        jrst    kret_neg1
        move    5,016(1)                 ; versioned handoff marker
        came    5,[044066263602]
        jrst    kret_neg1

        ; The transient scanner owns full filesystem validation.  Resident
        ; code checks only fields needed to keep provider/backing dispatch safe.
        move    5,020(1)                 ; backing.blocks / total capacity
        jumpe   5,kret_neg1
        came    5,6(1)                   ; super.total_blocks must agree
        jrst    kret_neg1
        skipe   3(1)                     ; secondary mounts must be clean
        jrst    kret_neg1

        push    17,1                     ; handoff
        push    17,2                     ; target vnode
        push    17,[0]                   ; allocated reader base
        movei   5,(17)                   ; mm_alloc basep -> local stack word
        push    17,5                     ; fifth mm_alloc arg
        movei   1,021                    ; struct d6fs_reader
        movei   2,3                      ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,010                    ; D6FS_READER_MM_OWNER
        setz    4,                       ; no alignment requirement
        pushj   17,mm_alloc
        sub     17,[1,,1]
        jumpn   1,d6fs_mount_bad

        move    5,(17)                   ; new mount-owned reader
        move    4,5                      ; BLT handoff into dynamic reader
        hrl     4,-2(17)
        blt     4,020(5)

        movei   6,016(5)                 ; temporary root-vnode scratch
        push    17,6                     ; sixth arg: rootp
        push    17,[1]                   ; fifth arg: VFS_MOUNT_RDONLY
        move    1,-3(17)                 ; target vnode
        movei   2,6                      ; D6FS_PROVIDER
        movei   3,1                      ; D6FS_KIND_NODE
        move    4,7(5)                   ; root_fcb
        pushj   17,vfs_mount_prevalidated
        sub     17,[2,,2]
        jumpn   1,d6fs_mount_free

        move    5,(17)
        movem   5,d6fs_reader_slots(7)
        movem   5,d6fs_active_reader
        move    6,1(5)                   ; prepacked summary/copy state
        move    4,7                      ; mount helper leaves zero-based slot AC7
        addi    4,1                      ; public mount id
        ior     6,4
        movem   6,1(5)
        move    4,[fs_backing_direct_read,,fs_backing_direct_write]
        movem   4,016(5)                 ; replace root scratch with trusted ops
        setz    1,
        jrst    d6fs_mount_done

d6fs_mount_free:
        move    1,(17)                   ; new reader was never installed
        movei   2,3
        movei   3,010
        pushj   17,mm_free
        seto    1,
        jrst    d6fs_mount_done

d6fs_mount_bad:
        seto    1,
d6fs_mount_done:
        sub     17,[3,,3]
        popj    17,

d6fs_mres_create:
        movei   5,1                     ; regular file type
        trnn    3,010000                ; private VFS CREATE-as-FIFO marker
        jrst    d6fs_mres_create_common
        andi    3,07777                 ; leave only persistent mode bits
        movei   5,4                     ; FIFO type
        jrst    d6fs_mres_create_common

d6fs_mres_mkdir:
        movei   5,2                     ; directory type
d6fs_mres_create_common:
        move    6,4                     ; nodep
        move    4,3                     ; mode => value
        setz    3,                      ; no payload
        jrst    d6fs_mres_create_call

d6fs_mres_symlink:
        move    6,4                     ; nodep
        setz    4,                      ; symlink mode/value unused
        movei   5,3                     ; symlink type
d6fs_mres_create_call:
        add     17,[2,,2]
        movem   5,(17)                  ; C arg 5: type
        movem   6,-1(17)                ; C arg 6: nodep
        pushj   17,d6fs_provider_create_object
        sub     17,[2,,2]
        popj    17,

        .data
d6fs_mres_vector:
        .word   d6fs_provider_lookup,,d6fs_provider_readdir
        .word   d6fs_provider_stat,,d6fs_provider_parent
        .word   d6fs_provider_parent_name,,d6fs_mres_create
        .word   d6fs_mres_mkdir,,d6fs_mres_symlink
        .word   d6fs_provider_unlink,,d6fs_provider_rename
        .word   d6fs_provider_truncate,,d6fs_provider_chmod
        .word   d6fs_provider_read_words,,d6fs_provider_write_words
        .word   d6fs_provider_sync,,d6fs_provider_prepare_unmount
        .text

; int d6fs_reader_read_words(reader, fcb, off, buf, nwords)
; int d6fs_reader_write_words(reader, fcb, off, buf, nwords)
;
; Both operations share block mapping and transfer-size calculation.  A saved
; tail address selects read copying or write copying/commit after each mapped
; block, avoiding a per-block read/write mode test.
        .globl  d6fs_reader_read_words
d6fs_reader_read_words:
        jumpe   1,kret_neg1
        jumpe   2,kret_neg1
        jumpe   4,kret_neg1
        move    5,2(2)                   ; file size in words
        caml    3,5                      ; off >= size
        jrst    kret_zero
        movei   0,d6fs_reader_read_transfer
        jrst    d6fs_reader_rw_save

        .globl  d6fs_reader_write_words
d6fs_reader_write_words:
        jumpe   1,kret_neg1
        jumpe   2,kret_neg1
        jumpe   4,kret_neg1
        movei   0,d6fs_reader_write_transfer

; Save the common loop state plus one transfer-tail address.  The original
; fifth C argument is therefore nine words below the resulting stack top.
d6fs_reader_rw_save:
        add     17,[7,,7]
        movei   7,-6(17)
        hrli    7,010
        blt     7,(17)
        push    17,0                     ; transfer tail
        move    010,1                    ; reader
        move    011,2                    ; fcb
        move    012,3                    ; base file offset
        move    013,4                    ; source/destination buffer
        move    014,-011(17)             ; original fifth arg: nwords
        setz    015,                     ; done
        caie    0,d6fs_reader_read_transfer
        jrst    d6fs_reader_rw_loop
        sub     5,012                    ; read available = size - off
        camle   014,5
        move    014,5                    ; nwords = min(nwords, available)

d6fs_reader_rw_loop:
        caml    015,014                  ; continue while done < nwords
        jrst    d6fs_reader_rw_done
        move    016,012
        add     016,015                  ; pos = off + done
        move    2,016
        lsh     2,-7                     ; file block
        move    1,011
        pushj   17,d6fs_file_block
        jumpl   1,d6fs_reader_rw_fail  ; only negative value is invalid
        move    016,1                    ; logical block (needed by write)
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_reader_rw_fail
        move    7,012
        add     7,015                    ; recompute pos after helper call
        andi    7,0177                   ; offset within block
        movei   5,0200
        sub     5,7                      ; max words in this block
        move    4,014
        sub     4,015                    ; words remaining
        camle   5,4
        move    5,4                      ; take
        jrst    @(17)                   ; operation-specific transfer tail

d6fs_reader_read_transfer:
        add     1,7                      ; source in cached block
        move    2,013
        add     2,015                    ; destination address
        move    3,5
        pushj   17,fs_copy_words
        add     015,5
        jrst    d6fs_reader_rw_loop

d6fs_reader_write_transfer:
        move    1,013
        add     1,015                    ; source address
        movei   2,fs_block_workspace(7)  ; destination in cached block
        move    3,5
        pushj   17,fs_copy_words
        add     015,5                    ; consume take before commit clobbers AC5
        move    1,010
        move    2,016
        pushj   17,d6fs_reader_commit_cache
        jumpn   1,d6fs_reader_rw_fail
        jrst    d6fs_reader_rw_loop

d6fs_reader_rw_done:
        move    1,015
        jrst    d6fs_reader_rw_exit
d6fs_reader_rw_fail:
        seto    1,
d6fs_reader_rw_exit:
        pop     17,0

d6fs_restore7:
        pop     17,016
d6fs_restore6:
        pop     17,015
d6fs_restore5:
        pop     17,014
d6fs_restore4:
        pop     17,013
d6fs_restore3:
        pop     17,012
d6fs_restore2:
        pop     17,011
d6fs_restore1:
        pop     17,010
        popj    17,

        .globl  d6fs_reader_commit_cache
; int d6fs_reader_commit_cache(reader, logical)
d6fs_reader_commit_cache:
        jumpe   1,d6fs_reader_commit_bad
        move    4,1(1)                   ; packed mount state
        trnn    4,0100                    ; D6FS_PROVIDER_MOUNT_WRITABLE
        jrst    d6fs_reader_commit_invalidate
        caml    2,6(1)                   ; logical >= total_blocks
        jrst    d6fs_reader_commit_invalidate
        push    17,010
        push    17,011
        push    17,012
        move    010,1
        move    011,2
        move    012,1(010)
        andi    012,077
        iori    012,01000                ; BCACHE_SOURCE_D6FS
        lsh     012,030
        ior     012,011
        movei   1,016(010)               ; embedded struct fs_backing
        move    2,011
        movei   3,fs_block_workspace
        pushj   17,fs_backing_write
        jumpn   1,d6fs_reader_commit_fail_saved
        move    1,012
        movei   2,fs_block_workspace
        pushj   17,bcache_store          ; refresh only after write-through
        setz    1,
        jrst    d6fs_reader_commit_done
d6fs_reader_commit_fail_saved:
        seto    1,
d6fs_reader_commit_done:
        jrst    d6fs_restore3
d6fs_reader_commit_invalidate:
d6fs_reader_commit_bad:
        jrst    kret_neg1

        .globl  d6fs_reader_write_block
; int d6fs_reader_write_block(reader, logical, block)
d6fs_reader_write_block:
        jumpe   1,kret_neg1
        jumpe   3,kret_neg1
        caml    2,6(1)
        jrst    kret_neg1
        camn    3,[fs_block_workspace]
        jrst    d6fs_reader_write_block_commit
        movei   4,fs_block_workspace
        hrl     4,3
        blt     4,fs_block_workspace+0177
d6fs_reader_write_block_commit:
        jrst    d6fs_reader_commit_cache

        .globl  d6fs_reader_zero_block
; int d6fs_reader_zero_block(reader, logical)
d6fs_reader_zero_block:
        jumpe   1,kret_neg1
        caml    2,6(1)
        jrst    kret_neg1
        push    17,2                     ; zero helper clobbers AC2 to 0177
        pushj   17,fs_zero_block_workspace
        pop     17,2                     ; restore requested logical block
        jrst    d6fs_reader_commit_cache

; Compact D6FS provider metadata/growth helpers.  These are leaf-sized
; representation operations shared by the larger C policy paths.
        .globl  d6fs_provider_set_extent
; void d6fs_provider_set_extent(fcb, index, start, blocks)
; Internal helper: callers pass validated index/start/block values.
d6fs_provider_set_extent:
        subi    4,1                     ; encoded length is blocks - 1
        move    5,3
        lsh     5,014
        move    6,4
        andi    6,07777
        ior     5,6
        move    6,1
        addi    6,6
        add     6,2
        movem   5,(6)
        move    5,2
        lsh     5,2
        add     5,2                     ; shift = index * 5
        movei   6,037
        lsh     6,0(5)
        andca   6,5(1)                  ; clear prior high-length field
        ldb     4,[POINT 5,4,027]
        lsh     4,0(5)
        ior     6,4
        movem   6,5(1)
        popj    17,

        .globl  d6fs_provider_clear_extent
; void d6fs_provider_clear_extent(fcb, index)
; Internal helper: callers pass an index in [0,D6FS_EXTENTS).
d6fs_provider_clear_extent:
        move    3,1
        addi    3,6
        add     3,2
        setzm   (3)
        move    3,2
        lsh     3,2
        add     3,2
        movei   4,037
        lsh     4,0(3)
        andcam  4,5(1)
        popj    17,

        .globl  d6fs_provider_free_file_tail
; int d6fs_provider_free_file_tail(fcb, first_file_block)
d6fs_provider_free_file_tail:
        push    17,010
        push    17,011
        move    010,1
        move    011,2
d6fs_provider_free_tail_loop:
        move    1,010
        move    2,011
        pushj   17,d6fs_file_block
        camn    1,[-1]
        jrst    d6fs_provider_free_tail_ok
        move    2,1
        move    1,d6fs_active_reader
        setz    3,
        pushj   17,d6fs_freemap_set
        jumpn   1,d6fs_provider_free_tail_done
        aoja    011,d6fs_provider_free_tail_loop
d6fs_provider_free_tail_ok:
        setz    1,
d6fs_provider_free_tail_done:
        jrst    d6fs_restore2

; int d6fs_provider_dirent(vnode_t dir, unsigned int slot,
;     struct d6fs_dirent_info *di)
; Keep only fixed scratch objects and the two live input values on the stack.
; This avoids GCC's five-register save area on this heavily shared helper.
        .globl  d6fs_provider_dirent
d6fs_provider_dirent:
        jumpe   3,kret_neg1
        add     17,[043,,043]            ; FCB + info + raw + slot/di + arg5
        movem   2,-2(17)                 ; slot
        movem   3,-1(17)                 ; decoded dirent output
        movei   2,-042(17)               ; 020-word FCB scratch
        movei   3,-022(17)               ; 012-word decoded FCB info
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_dirent_fail
        move    1,-022(17)               ; info.type
        caie    1,2                      ; D6FS_TYPE_DIR
        jrst    d6fs_provider_dirent_fail
        move    3,-2(17)                 ; off = slot * 6
        imuli   3,6
        move    5,-013(17)               ; validated positive info.size_words
        caml    3,5                      ; off < size_words?
        jrst    d6fs_provider_dirent_eof
        sub     5,3
        caige   5,6                      ; malformed short final dirent?
        jrst    d6fs_provider_dirent_fail
        move    1,d6fs_active_reader
        movei   2,-042(17)
        movei   4,-010(17)               ; 6-word raw dirent scratch
        movei   5,6
        movem   5,(17)                   ; fifth argument: nwords
        pushj   17,d6fs_reader_read_words
        caie    1,6
        jrst    d6fs_provider_dirent_fail
        movei   1,-010(17)
        move    2,d6fs_active_reader
        move    2,011(2)                 ; super.fcb_count
        move    3,-1(17)
        pushj   17,d6fs_dirent_decode_valid
        jumpe   1,d6fs_provider_dirent_fail
        movei   1,1
        jrst    d6fs_provider_dirent_done
d6fs_provider_dirent_eof:
        setz    1,
        jrst    d6fs_provider_dirent_done
d6fs_provider_dirent_fail:
        seto    1,
d6fs_provider_dirent_done:
        sub     17,[043,,043]
        popj    17,

; Private provider helpers use the same 0/-1 convention as reader helpers.
; Keeping these in assembly avoids compiler result normalization and allows
; successful FCB validation to tail-call d6fs_reader_fcb directly.
        .globl  d6fs_provider_fcb
d6fs_provider_fcb:
        move    6,2
        move    7,3
        hrrz    2,1
        move    1,d6fs_active_reader
        move    3,6
        move    4,7
        jrst    d6fs_reader_fcb

; unsigned int d6fs_provider_vtype(unsigned int type)
; D6FS types reaching this helper are validated REG/DIR/SYMLINK/FIFO 1..4.
; XCT keeps the translation smaller than a branch chain/table.
        .globl  d6fs_provider_vtype
d6fs_provider_vtype:
        xct     d6fs_provider_vtype_ops-1(1)
        popj    17,
d6fs_provider_vtype_ops:
        movei   1,2                     ; REG -> VFS_TYPE_REG
        movei   1,1                     ; DIR -> VFS_TYPE_DIR
        movei   1,6                     ; SYMLINK -> VFS_TYPE_SYMLINK
        movei   1,7                     ; FIFO -> VFS_TYPE_FIFO

; int d6fs_provider_stat(vnode_t node, struct vfs_stat *st)
        .globl  d6fs_provider_stat
d6fs_provider_stat:
        jumpe   2,kret_neg1
        add     17,[013,,013]            ; info + st pointer
        movem   2,-012(17)
        movei   2,0
        movei   3,-011(17)
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_stat_fail
        skipn   1,-011(17)
        jrst    d6fs_provider_stat_fail
        pushj   17,d6fs_provider_vtype
        move    2,-012(17)
        movem   1,(2)                    ; st->type
        move    1,-7(17)
        movem   1,1(2)                   ; st->mode
        setzm   2(2)                     ; reserved
        move    1,-2(17)
        movem   1,3(2)                   ; st->size_words
        move    1,-4(17)
        movem   1,4(2)                   ; st->uid
        move    1,-3(17)
        movem   1,5(2)                   ; st->gid
        move    1,-1(17)
        movem   1,6(2)                   ; st->mtime
        setz    1,
        jrst    d6fs_provider_stat_done
d6fs_provider_stat_fail:
        seto    1,
d6fs_provider_stat_done:
        sub     17,[013,,013]
        popj    17,

; int d6fs_provider_read_words(vnode_t node, unsigned int off,
;     kword_t *buf, unsigned int nwords)
; Validate into one compact FCB/info frame and leave nwords in the outgoing
; fifth-argument slot for d6fs_reader_read_words.
        .globl  d6fs_provider_read_words
d6fs_provider_read_words:
        jumpe   3,kret_neg1
        add     17,[035,,035]            ; FCB + info + off/buf/nwords
        movem   2,-2(17)                 ; off
        movem   3,-1(17)                 ; buf
        movem   4,(17)                   ; outgoing arg 5: nwords
        movei   2,-034(17)               ; 020-word FCB scratch
        movei   3,-014(17)               ; 012-word decoded info
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_read_words_fail
        move    1,-014(17)               ; info.type
        cain    1,1                      ; D6FS_TYPE_REG
        jrst    d6fs_provider_read_words_ok
        caie    1,3                      ; D6FS_TYPE_SYMLINK
        jrst    d6fs_provider_read_words_fail
d6fs_provider_read_words_ok:
        move    1,d6fs_active_reader
        movei   2,-034(17)
        move    3,-2(17)
        move    4,-1(17)
        pushj   17,d6fs_reader_read_words
        jrst    d6fs_provider_read_words_done
d6fs_provider_read_words_fail:
        seto    1,
d6fs_provider_read_words_done:
        sub     17,[035,,035]
        popj    17,

; int d6fs_provider_sync(vnode_t node)
        .globl  d6fs_provider_sync
d6fs_provider_sync:
        jrst    kret_zero

; Publish the opposite A/B superblock and toggle CLEAN/DIRTY state.
; All D6FS writes are synchronous and the dynamic cache is clean-only, so this
; publication is the complete persistent barrier for RO/RW transitions.
d6fs_provider_toggle_state:
        move    5,d6fs_active_reader
        move    4,1(5)
        move    2,015(5)                 ; current A -> publish B
        trne    4,0200                    ; current B -> publish A
        move    2,014(5)
        push    17,2
        move    1,5
        pushj   17,d6fs_reader_get_block
        pop     17,2
        jumpe   1,kret_neg1
        move    5,d6fs_active_reader
        move    4,2(5)
        addi    4,1
        movem   4,fs_block_workspace+1   ; sequence
        ldb     4,[POINT 1,1(5),29]      ; current writable bit
        xori    4,1                      ; media state: RW->CLEAN, RO->DIRTY
        movem   4,fs_block_workspace+2
        move    1,5
        movei   3,fs_block_workspace
        pushj   17,d6fs_reader_write_block
        jumpn   1,kret_neg1
        move    5,d6fs_active_reader
        aos     2(5)
        movei   4,0300                   ; WRITABLE | COPY
        xorm    4,1(5)
        ldb     4,[POINT 6,1(5),35]      ; mount id
        movei   6,1
        lsh     6,-1(4)
        xorm    6,vfs_mount_ro           ; VFS policy changes after publication
        jrst    kret_zero

; int d6fs_provider_prepare_unmount(vnode_t root)
        .globl  d6fs_provider_prepare_unmount
d6fs_provider_prepare_unmount:
        move    5,d6fs_active_reader
        move    4,1(5)
        trnn    4,0100                    ; D6FS_PROVIDER_MOUNT_WRITABLE
        jrst    d6fs_provider_unmount_done
        pushj   17,d6fs_provider_toggle_state
        jumpn   1,kret_neg1
d6fs_provider_unmount_done:
        setz    1,
        pushj   17,bcache_reclaim        ; source id may be reused after unmount
        move    5,d6fs_active_reader
        ldb     4,[POINT 6,1(5),35]      ; mount id
        subi    4,1                      ; VFS mount ids are 1..4
        push    17,4                     ; mm_free may clobber argument ACs
        move    1,5
        movei   2,3                      ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,010                    ; D6FS_READER_MM_OWNER
        pushj   17,mm_free
        jumpn   1,d6fs_provider_unmount_free_fail
        pop     17,4
        setzm   d6fs_reader_slots(4)
        setzm   d6fs_active_reader
        jrst    kret_zero
d6fs_provider_unmount_free_fail:
        pop     17,4
        jrst    kret_neg1

        .globl  pclk_time36

; int d6fs_provider_chmod(vnode_t node, unsigned int mode)
; The VFS also uses this provider slot as a private compact setattr channel:
;   AC2 0100000, AC3 UID9,,GID9 -> CHOWN
;   AC2 0100001, AC3 TIME36    -> UTIME
; Ordinary chmod keeps AC2 <= 07777.  One implementation avoids extra MRES
; vector entries while keeping the external VFS operations distinct.
        .globl  d6fs_provider_chmod
d6fs_provider_chmod:
        add     17,[035,,035]            ; FCB + info + node/cmd/value
        movem   1,-2(17)                 ; node
        movem   2,-1(17)                 ; mode or private command
        movem   3,(17)                   ; private value
        movei   2,-034(17)               ; 020-word FCB scratch
        movei   3,-014(17)               ; 012-word decoded info
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_chmod_fail
        skipn   -014(17)                 ; FREE FCB
        jrst    d6fs_provider_chmod_fail
        move    4,-013(17)               ; info.flags
        trne    4,020                     ; D6FS_FLAG_IMMUTABLE
        jrst    d6fs_provider_chmod_fail
        move    5,-1(17)
        cain    5,0100000
        jrst    d6fs_provider_chown_store
        cain    5,0100001
        jrst    d6fs_provider_utime_store
        caile   5,07777
        jrst    d6fs_provider_chmod_fail
        move    4,-034(17)               ; FCB META
        dpb     5,[POINT 12,4,23]        ; replace mode bits 12..23
        movem   4,-034(17)
        jrst    d6fs_provider_attr_commit
d6fs_provider_chown_store:
        move    4,(17)                    ; compact UID9,,GID9
        move    5,4
        lsh     5,-011
        lsh     5,022                     ; UID -> FCB OWNER LH
        andi    4,0777                    ; GID -> FCB OWNER RH
        ior     4,5
        movem   4,-033(17)                ; preserve D6FS V2 on-media layout
        jrst    d6fs_provider_attr_commit
d6fs_provider_utime_store:
        move    4,(17)
        movem   4,-031(17)               ; FCB MTIME (word 3)
d6fs_provider_attr_commit:
        move    1,d6fs_active_reader
        hrrz    2,-2(17)
        movei   3,-034(17)
        pushj   17,d6fs_reader_put_fcb
        jrst    d6fs_provider_chmod_done
d6fs_provider_chmod_fail:
        seto    1,
d6fs_provider_chmod_done:
        sub     17,[035,,035]
        popj    17,

; int d6fs_provider_truncate(vnode_t node, unsigned int words)
        .globl  d6fs_provider_resize_fcb
        .globl  d6fs_provider_truncate
d6fs_provider_truncate:
        add     17,[035,,035]
        movem   1,-2(17)                 ; node
        movem   2,-1(17)                 ; words
        movei   2,-034(17)
        movei   3,-014(17)
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_truncate_fail
        move    1,-014(17)
        caie    1,1                      ; regular files only
        jrst    d6fs_provider_truncate_fail
        move    4,-013(17)
        trne    4,021                    ; APPEND | IMMUTABLE
        jrst    d6fs_provider_truncate_fail
        move    1,-2(17)
        movei   2,-034(17)
        movei   3,-014(17)
        move    4,-1(17)
        pushj   17,d6fs_provider_resize_fcb
        jumpn   1,d6fs_provider_truncate_fail
        pushj   17,pclk_time36
        movem   1,-031(17)
        move    1,d6fs_active_reader
        hrrz    2,-2(17)
        movei   3,-034(17)
        pushj   17,d6fs_reader_put_fcb
        jrst    d6fs_provider_truncate_done
d6fs_provider_truncate_fail:
        seto    1,
d6fs_provider_truncate_done:
        sub     17,[035,,035]
        popj    17,

; int d6fs_provider_lookup(vnode_t dir, const struct vfs_name *name,
;     vnode_t *nodep)
; Keep only the output pointer and directory vnode across the directory scan.
; The 010-word scratch area is one decoded d6fs_dirent_info.
        .globl  d6fs_provider_scan_slot
        .globl  d6fs_provider_lookup
d6fs_provider_lookup:
        jumpe   3,kret_neg1
        add     17,[012,,012]
        movem   1,-1(17)                 ; directory vnode
        movem   3,(17)                   ; output pointer
        movei   3,0
        movei   4,-011(17)               ; decoded dirent scratch
        pushj   17,d6fs_provider_scan_slot
        jumpn   1,d6fs_provider_lookup_fail
        hrrz    1,-2(17)                 ; di.child_fcb
        tlo     1,060001                 ; provider 6, local NODE kind
        move    2,(17)
        movem   1,(2)
        setz    1,
        jrst    d6fs_provider_lookup_done
d6fs_provider_lookup_fail:
        seto    1,
d6fs_provider_lookup_done:
        sub     17,[012,,012]
        popj    17,

; int d6fs_provider_parent(vnode_t node, vnode_t *parentp)
; Keep the vnode and destination beside one decoded FCB-info scratch area.
        .globl  d6fs_provider_parent
d6fs_provider_parent:
        jumpe   2,kret_neg1
        add     17,[014,,014]
        movem   1,-1(17)                 ; node
        movem   2,(17)                   ; parentp
        movei   2,0
        movei   3,-013(17)               ; 012-word decoded FCB info
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_parent_fail
        hrrz    4,-1(17)                ; node index
        move    5,d6fs_active_reader
        camn    4,7(5)                   ; root_fcb: root is its own parent
        jrst    d6fs_provider_parent_build
        move    4,-2(17)                 ; fi.parent_fcb
        caml    4,011(5)                 ; reject parent outside FCB table
        jrst    d6fs_provider_parent_fail
d6fs_provider_parent_build:
        move    1,4
        tlo     1,060001                 ; provider 6, local NODE kind
d6fs_provider_parent_store:
        move    2,(17)
        movem   1,(2)
        setz    1,
        jrst    d6fs_provider_parent_done
d6fs_provider_parent_fail:
        seto    1,
d6fs_provider_parent_done:
        sub     17,[014,,014]
        popj    17,

; int d6fs_provider_write_words(vnode_t node, unsigned int off,
;     const kword_t *buf, unsigned int nwords)
        .globl  d6fs_provider_write_words
        .globl  d6fs_reader_write_words
d6fs_provider_write_words:
        jumpe   3,kret_neg1
        add     17,[037,,037]
        movem   1,-4(17)                 ; node
        movem   2,-3(17)                 ; off
        movem   3,-2(17)                 ; buf
        movem   4,-1(17)                 ; nwords
        movei   2,-036(17)
        movei   3,-016(17)
        pushj   17,d6fs_provider_fcb
        jumpn   1,d6fs_provider_write_words_fail
        move    1,-016(17)
        caie    1,1                      ; regular files only
        jrst    d6fs_provider_write_words_fail
        move    4,-015(17)
        trne    4,020                    ; D6FS_FLAG_IMMUTABLE
        jrst    d6fs_provider_write_words_fail
        trnn    4,1                      ; D6FS_FLAG_APPEND
        jrst    d6fs_provider_write_words_append_ok
        move    5,-3(17)
        came    5,-7(17)                 ; append requires off == old size
        jrst    d6fs_provider_write_words_fail
d6fs_provider_write_words_append_ok:
        move    5,-3(17)
        add     5,-1(17)                 ; need = off + nwords
        camle   5,-7(17)                 ; need <= old size: overwrite only
        jrst    d6fs_provider_write_words_resize
        jrst    d6fs_provider_write_words_store
d6fs_provider_write_words_resize:
        move    1,-4(17)
        movei   2,-036(17)
        movei   3,-016(17)
        move    4,5
        pushj   17,d6fs_provider_resize_fcb
        jumpn   1,d6fs_provider_write_words_fail
d6fs_provider_write_words_store:
        move    5,-1(17)
        movem   5,(17)                   ; fifth arg: nwords
        move    1,d6fs_active_reader
        movei   2,-036(17)
        move    3,-3(17)
        move    4,-2(17)
        pushj   17,d6fs_reader_write_words
        jumpl   1,d6fs_provider_write_words_fail
        movem   1,(17)
        pushj   17,pclk_time36
        movem   1,-033(17)
        move    1,d6fs_active_reader
        hrrz    2,-4(17)
        movei   3,-036(17)
        pushj   17,d6fs_reader_put_fcb
        jumpn   1,d6fs_provider_write_words_fail
        move    1,(17)
        jrst    d6fs_provider_write_words_done
d6fs_provider_write_words_fail:
        seto    1,
d6fs_provider_write_words_done:
        sub     17,[037,,037]
        popj    17,

; int d6fs_provider_readdir(vnode_t dir, unsigned int off,
;     struct vfs_dirent *ent)
; Keep scan state in a compact stack frame and reuse provider_dirent's decoded
; entry directly.  AC1-AC7 are call-clobbered, so no register save block is
; required around the helper calls.
        .globl  d6fs_provider_readdir
d6fs_provider_readdir:
        jumpe   3,kret_neg1
        add     17,[015,,015]            ; di[8] + dir/off/ent/slot/seen
        movem   1,-4(17)
        movem   2,-3(17)
        movem   3,-2(17)
        setzm   -1(17)                   ; slot
        setzm   (17)                     ; seen
d6fs_provider_readdir_loop:
        move    1,-4(17)
        move    2,-1(17)
        movei   3,-014(17)               ; struct d6fs_dirent_info
        pushj   17,d6fs_provider_dirent
        jumpe   1,d6fs_provider_readdir_eof
        jumpl   1,d6fs_provider_readdir_fail
        skipn   -5(17)                   ; di.child_fcb
        jrst    d6fs_provider_readdir_next
        move    4,(17)
        aos     (17)
        came    4,-3(17)
        jrst    d6fs_provider_readdir_next
        movei   1,-014(17)               ; di.name
        move    2,-2(17)                 ; ent->name
        pushj   17,vfs_name_from_words
        move    1,-7(17)                 ; di.type
        pushj   17,d6fs_provider_vtype
        move    2,-2(17)
        movem   1,5(2)                   ; ent->type
        movei   1,1
        jrst    d6fs_provider_readdir_done
d6fs_provider_readdir_next:
        aos     -1(17)
        jrst    d6fs_provider_readdir_loop
d6fs_provider_readdir_eof:
        setz    1,
        jrst    d6fs_provider_readdir_done
d6fs_provider_readdir_fail:
        seto    1,
d6fs_provider_readdir_done:
        sub     17,[015,,015]
        popj    17,

; int d6fs_provider_parent_name(vnode_t node, vnode_t *parentp,
;     struct vfs_name *namep)
        .globl  d6fs_provider_parent_name
d6fs_provider_parent_name:
        jumpe   2,kret_neg1
        jumpe   3,kret_neg1
        add     17,[015,,015]            ; di[8] + node/parentp/namep/slot/parent
        movem   1,-4(17)
        movem   2,-3(17)
        movem   3,-2(17)
        movei   2,(17)                   ; parent scratch
        pushj   17,d6fs_provider_parent
        jumpn   1,d6fs_provider_parent_name_fail
        hrrz    4,(17)
        hrrz    5,-4(17)
        camn    4,5                      ; root/self has no parent name
        jrst    d6fs_provider_parent_name_fail
        setzm   -1(17)                   ; slot
d6fs_provider_parent_name_loop:
        move    1,(17)
        move    2,-1(17)
        movei   3,-014(17)               ; struct d6fs_dirent_info
        pushj   17,d6fs_provider_dirent
        jumple  1,d6fs_provider_parent_name_fail
        move    4,-5(17)                 ; di.child_fcb
        hrrz    5,-4(17)                 ; VFS_INDEX(node)
        came    4,5
        jrst    d6fs_provider_parent_name_next
        movei   1,-014(17)               ; di.name
        move    2,-2(17)                 ; namep
        pushj   17,vfs_name_from_words
        move    1,(17)
        move    2,-3(17)
        movem   1,(2)
        setz    1,
        jrst    d6fs_provider_parent_name_done
d6fs_provider_parent_name_next:
        aos     -1(17)
        jrst    d6fs_provider_parent_name_loop
d6fs_provider_parent_name_fail:
        seto    1,
d6fs_provider_parent_name_done:
        sub     17,[015,,015]
        popj    17,

        .bss
