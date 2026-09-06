; d6fs_pdp10.s -- compact resident D6FS packed-format primitives.
;
; These routines replace C shift/divide machinery on the PDP-6/PDP-10 hot
; paths.  They implement the on-disk D6FS V2 bit layout directly; policy and
; crash-ordering remain in C.
        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

; FCB decode/validation is implemented in d6fs.c.

        .globl  d6fs_dirent_decode_valid
; int d6fs_dirent_decode_valid(ent, fcb_count, info)
d6fs_dirent_decode_valid:
        jumpe   1,d6fs_dirent_invalid
        jumpe   2,d6fs_dirent_invalid
        jumpe   3,d6fs_dirent_invalid
        move    4,5(1)
        trne    4,0777777                ; low half must be zero
        jrst    d6fs_dirent_invalid
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
        jrst    d6fs_dirent_invalid

d6fs_dirent_used:
        caml    4,2                      ; child_fcb >= fcb_count
        jrst    d6fs_dirent_invalid
        jumpe   6,d6fs_dirent_invalid    ; FREE type forbidden
        caile   6,3
        jrst    d6fs_dirent_invalid
        move    4,(1)
        ior     4,1(1)
        ior     4,2(1)
        ior     4,3(1)
        jumpe   4,d6fs_dirent_invalid

d6fs_dirent_valid:
        movei   1,1
        popj    17,
d6fs_dirent_invalid:
        jrst    pdp10_ret_zero

        .globl  d6fs_extent_decode
; int d6fs_extent_decode(run, high, startp, blocksp)
d6fs_extent_decode:
        jumpe   3,d6fs_extent_bad
        jumpe   4,d6fs_extent_bad
        tdne    2,[-040]                 ; high must fit five bits
        jrst    d6fs_extent_bad
        ldb     5,[POINT 24,1,23]
        movem   5,(3)
        lsh     2,014
        andi    1,07777
        ior     2,1
        addi    2,1
        movem   2,(4)
        jrst    pdp10_ret_zero
d6fs_extent_bad:
        jrst    pdp10_ret_neg1

        .globl  d6fs_extent_high_get
; unsigned int d6fs_extent_high_get(word, extent)
d6fs_extent_high_get:
        caile   2,6
        jrst    d6fs_high_zero
        move    3,2
        lsh     3,2
        add     3,2                      ; shift = extent * 5
        movn    3,3
        lsh     1,0(3)
        andi    1,037
        popj    17,
d6fs_high_zero:
        jrst    pdp10_ret_zero

; kword_t d6fs_file_block(fcb, file_block)
;
; Return the logical block directly; D6FS_CACHE_INVALID (-1) means that the
; requested file block is outside all extents.  Logical D6FS block numbers are
; only 24 bits, so the sentinel cannot collide with valid media.
        .globl  d6fs_file_block
d6fs_file_block:
        jumpe   1,d6fs_file_block_bad
        move    3,(1)
        lsh     3,-4
        andi    3,017                    ; extent_count
        jumpe   3,d6fs_file_block_bad
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

d6fs_file_block_bad:
        jrst    pdp10_ret_neg1

        .globl  d6fs_name_hash24
; kword_t d6fs_name_hash24(words, chars)
;
; Hash packed SIXBIT directly with ILDB; this avoids C division/modulo and
; per-character word shifting.  SIXBIT bytes are converted to ASCII by +040.
d6fs_name_hash24:
        jumpe   1,d6fs_hash_zero
        jumpe   2,d6fs_hash_zero
        caile   2,030                    ; maximum 24 characters
        jrst    d6fs_hash_zero
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

d6fs_hash_zero:
        jrst    pdp10_ret_zero

        .globl  fs_block_workspace
        .globl  d6fs_reader_get_block
; const kword_t *d6fs_reader_get_block(reader, logical)
;
; A successful load always exposes reader->cache, so return that address
; directly instead of forcing every caller to allocate a pointer temporary.
d6fs_reader_get_block:
        jumpe   1,d6fs_get_block_fail
        caml    2,6(1)                   ; logical >= total_blocks
        jrst    d6fs_get_block_fail
        camn    2,3(1)                   ; cache hit
        jrst    d6fs_get_block_hit
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        movei   3,fs_block_workspace     ; shared transfer block
        move    1,1(010)                 ; opaque
        move    2,011
        pushj   17,d6fs_diskset_read     ; fixed MINIT-patched DISKSET read
        jumpn   1,d6fs_get_block_read_fail
        movem   011,3(010)
        movei   1,fs_block_workspace
        jrst    d6fs_get_block_read_done
d6fs_get_block_read_fail:
        setz    1,
d6fs_get_block_read_done:
        pop     17,011
        pop     17,010
        popj    17,
d6fs_get_block_hit:
        movei   1,fs_block_workspace
        popj    17,
d6fs_get_block_fail:
        jrst    pdp10_ret_zero

        .globl  d6fs_reader_fcb
; int d6fs_reader_fcb(reader, index, fcb, info)
;
; FCBs are fixed 16-word records and 128-word blocks therefore contain exactly
; eight aligned FCBs.  Use shifts/masks and BLT instead of compiler division
; and a sixteen-iteration copy loop.
d6fs_reader_fcb:
        jumpe   1,d6fs_reader_fcb_bad
        jumpe   4,d6fs_reader_fcb_bad
        caml    2,011(1)                 ; index >= fcb_count
        jrst    d6fs_reader_fcb_bad
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
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
d6fs_reader_fcb_bad:
        jrst    pdp10_ret_neg1

        .globl  d6fs_reader_put_fcb
; int d6fs_reader_put_fcb(reader, index, fcb)
d6fs_reader_put_fcb:
        jumpe   1,d6fs_reader_put_fcb_bad
        jumpe   3,d6fs_reader_put_fcb_bad
        caml    2,011(1)
        jrst    d6fs_reader_put_fcb_bad
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
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
d6fs_reader_put_fcb_bad:
        jrst    pdp10_ret_neg1

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
        jumpe   1,d6fs_bitmap_error
        caml    2,6(1)
        jrst    d6fs_bitmap_error
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
        setz    1,
        tdne    2,1
        movei   1,1
        jrst    d6fs_freemap_state_done
d6fs_freemap_state_fail:
        seto    1,
d6fs_freemap_state_done:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .globl  d6fs_free_run
; int d6fs_free_run(reader, start, blocks)
; Clear one contiguous allocation run.  Keep this with the bitmap primitives so
; callers do not pay a compiler-generated loop around d6fs_freemap_set().
d6fs_free_run:
        jumpe   1,d6fs_free_run_error
        jumpe   3,d6fs_free_run_error
        caml    2,6(1)                   ; start >= total_blocks
        jrst    d6fs_free_run_error
        move    4,6(1)
        sub     4,2                      ; blocks available from start
        camle   3,4
        jrst    d6fs_free_run_error
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
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
d6fs_free_run_error:
        jrst    pdp10_ret_neg1

        .globl  d6fs_alloc_run
; int d6fs_alloc_run(reader, cursor, max_blocks, startp, blocksp)
;
; Selection only: this routine never changes the free map.  The caller writes
; new data first, then commits allocation bits, then persists FCB reachability.
; Return the first contiguous free run at or after cursor, limited to max_blocks.
d6fs_alloc_run:
        jumpe   1,d6fs_alloc_run_error
        jumpe   3,d6fs_alloc_run_error
        jumpe   4,d6fs_alloc_run_error
        skipn   6(1)                     ; total_blocks
        jrst    d6fs_alloc_run_error
        skipn   5,-1(17)                 ; fifth C arg: blocksp
        jrst    d6fs_alloc_run_error
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                    ; reader
        move    011,2                    ; cursor
        move    012,3                    ; max_blocks
        move    013,4                    ; startp
        move    014,5                    ; blocksp
        move    015,6(1)                 ; total_blocks
        move    1,011
        idiv    1,015
        move    011,2                    ; cursor %= total_blocks
        setz    016,                      ; scanned
        add     17,[3,,3]                ; local start, count, current
        setzm   (17)                     ; count
        setzm   -1(17)                   ; start

d6fs_alloc_run_loop:
        caml    016,015
        jrst    d6fs_alloc_run_end
        move    2,011
        add     2,016                    ; logical = cursor + scanned
        caml    2,015
        sub     2,015                    ; one wrap is sufficient
        movem   2,-2(17)                ; preserve current across helper
        move    1,010
        pushj   17,d6fs_freemap_state
        jumpl   1,d6fs_alloc_run_fail
        jumpn   1,d6fs_alloc_run_used
        move    2,(17)                   ; count
        jumpn   2,d6fs_alloc_run_continue
        move    3,-2(17)
        movem   3,-1(17)                 ; first free logical block
        jrst    d6fs_alloc_run_add
d6fs_alloc_run_continue:
        move    3,-1(17)
        add     3,2
        move    4,-2(17)                ; current logical block
        came    4,3                      ; do not join across wrap
        jrst    d6fs_alloc_run_restart
d6fs_alloc_run_add:
        aos     (17)
        move    2,(17)
        camge   2,012
        jrst    d6fs_alloc_run_success
        aoja    016,d6fs_alloc_run_loop
d6fs_alloc_run_restart:
        movem   4,-1(17)
        movei   2,1
        movem   2,(17)
        camge   2,012
        jrst    d6fs_alloc_run_success
        aoja    016,d6fs_alloc_run_loop
d6fs_alloc_run_used:
        skipn   (17)
        aoja    016,d6fs_alloc_run_loop
        jrst    d6fs_alloc_run_success
d6fs_alloc_run_end:
        skipn   (17)
        jrst    d6fs_alloc_run_fail
d6fs_alloc_run_success:
        move    1,-1(17)
        movem   1,(013)
        move    1,(17)
        movem   1,(014)
        setz    1,
        jrst    d6fs_alloc_run_done
d6fs_alloc_run_fail:
        seto    1,
d6fs_alloc_run_done:
        sub     17,[3,,3]
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
d6fs_alloc_run_error:
        jrst    pdp10_ret_neg1

; Internal: return 1 if map block has a free valid bit, 0 if full, -1 error.
; Scan whole 36-bit words instead of testing as many as 4608 individual bits.
d6fs_map_block_has_free_i:
        jumpe   1,d6fs_bitmap_error
        caml    2,013(1)
        jrst    d6fs_bitmap_error
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
        jumpe   1,d6fs_map_scan_fail
        move    2,012
        idivi   2,044                     ; AC2 full words, AC3 remainder
        move    4,1                       ; current bitmap word
        move    5,2
        jumpe   5,d6fs_map_scan_partial

d6fs_map_scan_words:
        move    6,(4)
        came    6,[-1]
        jrst    d6fs_map_scan_free
        addi    4,1
        sojg    5,d6fs_map_scan_words

d6fs_map_scan_partial:
        jumpe   3,d6fs_map_scan_full
        movei   5,044
        sub     5,3                       ; 36 - remainder
        seto    6,
        lsh     6,0(5)                    ; mask valid MSB-first bits
        move    7,(4)
        and     7,6
        came    7,6
        jrst    d6fs_map_scan_free

d6fs_map_scan_full:
        jrst    pdp10_ret_zero
d6fs_map_scan_free:
        movei   1,1
        popj    17,
d6fs_map_scan_fail:
        jrst    pdp10_ret_neg1

; Internal summary bit setter: reader AC1, map index AC2, boolean AC3.
d6fs_summary_set_i:
        ; Internal tail helper for d6fs_freemap_set.  The caller has already
        ; saved AC10-AC14 and does not use their working values afterwards.
        move    010,1
        move    014,3
        move    1,2
        pushj   17,d6fs_bitmap_pos
        caml    1,015(010)
        jrst    d6fs_summary_set_fail
        move    011,1                     ; summary block index
        move    012,2                     ; word index
        move    013,3                     ; bit index
        add     1,014(010)
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_summary_set_fail
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
        add     2,014(010)
        move    1,010
        jrst    d6fs_reader_commit_cache
d6fs_summary_set_fail:
        jrst    pdp10_ret_neg1

        .globl  d6fs_freemap_set
; int d6fs_freemap_set(reader, logical, allocated)
d6fs_freemap_set:
        jumpe   1,d6fs_bitmap_error
        caml    2,6(1)
        jrst    d6fs_bitmap_error
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
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
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

d6fs_bitmap_error:
        jrst    pdp10_ret_neg1

        .globl  fs_copy_words

        .globl  d6fs_diskset_read
        .globl  d6fs_diskset_read_jump
; Reader callbacks use (opaque, logical, block).  DISKSET block I/O uses
; (logical, block).  MINIT patches the tail jump once at boot.
d6fs_diskset_read:
        move    1,2
        move    2,3
d6fs_diskset_read_jump:
        jrst    0

        .globl  d6fs_diskset_write
        .globl  d6fs_diskset_write_jump
d6fs_diskset_write:
        move    1,2
        move    2,3
d6fs_diskset_write_jump:
        jrst    0

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
        move    2,[d6fs_mres_vector]
        jrst    fs_mres_vector_dispatch

d6fs_mres_create:
        movei   5,1                     ; regular file type
        jrst    d6fs_mres_create_common

d6fs_mres_mkdir:
        movei   5,2                     ; directory type
d6fs_mres_create_common:
        move    6,4                     ; nodep
        move    4,3                     ; mode => value
        setz    3,                      ; no payload
        jrst    d6fs_mres_create_call

d6fs_mres_symlink:
        move    6,-1(17)                ; incoming C arg 5: nodep
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
        .word   020                      ; highest operation: 16 decimal
        .word   d6fs_provider_lookup     ; 1 LOOKUP
        .word   d6fs_provider_readdir    ; 2 READDIR
        .word   d6fs_provider_stat       ; 3 STAT
        .word   d6fs_provider_parent     ; 4 PARENT
        .word   d6fs_provider_parent_name ; 5 PARENT_NAME
        .word   d6fs_mres_create         ; 6 CREATE
        .word   d6fs_mres_mkdir          ; 7 MKDIR
        .word   d6fs_mres_symlink        ; 8 SYMLINK
        .word   d6fs_provider_unlink     ; 9 UNLINK
        .word   d6fs_provider_rename     ; 10 RENAME
        .word   d6fs_provider_truncate   ; 11 TRUNCATE
        .word   d6fs_provider_chmod      ; 12 CHMOD
        .word   d6fs_provider_read_words ; 13 READ_WORDS
        .word   d6fs_provider_write_words ; 14 WRITE_WORDS
        .word   d6fs_provider_sync       ; 15 SYNC
        .word   d6fs_provider_prepare_unmount ; 16 PREPARE_UNMOUNT
        .text

; int d6fs_reader_read_words(reader, fcb, off, buf, nwords)
; Fifth C argument is at -1(17) on entry.  D6FS blocks are 128 words, so
; block division/modulo reduce to shift/mask.  BLT handles each contiguous
; in-block transfer.
        .globl  d6fs_reader_read_words
d6fs_reader_read_words:
        jumpe   1,d6fs_reader_read_bad
        jumpe   2,d6fs_reader_read_bad
        jumpe   4,d6fs_reader_read_bad
        move    5,2(2)                   ; file size in words
        caml    3,5                      ; off >= size
        jrst    d6fs_reader_read_zero
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                    ; reader
        move    011,2                    ; fcb
        move    012,3                    ; base file offset
        move    013,4                    ; destination
        move    014,-010(17)             ; original fifth arg: nwords
        sub     5,012                    ; available = size - off
        camle   014,5
        move    014,5                    ; nwords = min(nwords, available)
        setz    015,                     ; done

d6fs_reader_read_loop:
        caml    015,014                  ; continue while done < nwords
        jrst    d6fs_reader_read_done
        move    016,012
        add     016,015                  ; pos = off + done
        move    2,016
        lsh     2,-7                     ; file block
        move    1,011
        pushj   17,d6fs_file_block
        camn    1,[-1]
        jrst    d6fs_reader_read_fail
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_reader_read_fail
        move    6,016
        andi    6,0177                   ; offset within block
        add     1,6                      ; source address
        movei   7,0200
        sub     7,6                      ; max words in this block
        move    5,014
        sub     5,015                    ; words remaining
        camle   7,5
        move    7,5                      ; take = min(block remainder, total)
        jumpe   7,d6fs_reader_read_done
        move    2,013
        add     2,015                    ; destination address
        move    3,7                      ; count
        pushj   17,fs_copy_words
        add     015,7
        jrst    d6fs_reader_read_loop

d6fs_reader_read_done:
        move    1,015
        jrst    d6fs_reader_read_exit
d6fs_reader_read_fail:
        seto    1,
d6fs_reader_read_exit:
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
d6fs_reader_read_zero:
        jrst    pdp10_ret_zero
d6fs_reader_read_bad:
        jrst    pdp10_ret_neg1

        .globl  d6fs_reader_write_words
; int d6fs_reader_write_words(reader, fcb, off, buf, nwords)
; The caller has already grown the FCB as required.  This routine only maps
; file blocks, copies words into the shared cache, and commits each block.
d6fs_reader_write_words:
        jumpe   1,d6fs_reader_write_bad
        jumpe   2,d6fs_reader_write_bad
        jumpe   4,d6fs_reader_write_bad
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        move    010,1                    ; reader
        move    011,2                    ; fcb
        move    012,3                    ; base file offset
        move    013,4                    ; source buffer
        move    014,-010(17)             ; original fifth arg: nwords
        setz    015,                     ; done

d6fs_reader_write_loop:
        caml    015,014                  ; continue while done < nwords
        jrst    d6fs_reader_write_done
        move    016,012
        add     016,015                  ; pos = off + done
        move    2,016
        lsh     2,-7                     ; file block
        move    1,011
        pushj   17,d6fs_file_block
        camn    1,[-1]
        jrst    d6fs_reader_write_fail
        move    6,1                      ; logical block
        move    2,1
        move    1,010
        pushj   17,d6fs_reader_get_block
        jumpe   1,d6fs_reader_write_fail
        move    7,016
        andi    7,0177                   ; offset within block
        movei   5,0200
        sub     5,7                      ; max words in this block
        move    4,014
        sub     4,015                    ; words remaining
        camle   5,4
        move    5,4                      ; take
        jumpe   5,d6fs_reader_write_done
        move    1,013
        add     1,015                    ; source address
        movei   2,fs_block_workspace(7)  ; destination address
        move    3,5                      ; count
        pushj   17,fs_copy_words
        move    1,010
        move    2,6
        pushj   17,d6fs_reader_commit_cache
        jumpn   1,d6fs_reader_write_fail
        add     015,5
        jrst    d6fs_reader_write_loop

d6fs_reader_write_done:
        move    1,015
        jrst    d6fs_reader_write_exit
d6fs_reader_write_fail:
        seto    1,
d6fs_reader_write_exit:
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
d6fs_reader_write_bad:
        jrst    pdp10_ret_neg1

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
        move    010,1
        move    011,2
        move    1,1(010)                 ; opaque
        move    2,011
        movei   3,fs_block_workspace
        pushj   17,d6fs_diskset_write
        jumpn   1,d6fs_reader_commit_fail_saved
        movem   011,3(010)
        setz    1,
        jrst    d6fs_reader_commit_done
d6fs_reader_commit_fail_saved:
        setom   3(010)
        seto    1,
d6fs_reader_commit_done:
        pop     17,011
        pop     17,010
        popj    17,
d6fs_reader_commit_invalidate:
        setom   3(1)
d6fs_reader_commit_bad:
        jrst    pdp10_ret_neg1

        .globl  d6fs_reader_write_block
; int d6fs_reader_write_block(reader, logical, block)
d6fs_reader_write_block:
        jumpe   1,d6fs_reader_write_block_bad
        jumpe   3,d6fs_reader_write_block_bad
        caml    2,6(1)
        jrst    d6fs_reader_write_block_bad
        camn    3,[fs_block_workspace]
        jrst    d6fs_reader_write_block_commit
        movei   4,fs_block_workspace
        hrl     4,3
        blt     4,fs_block_workspace+0177
d6fs_reader_write_block_commit:
        jrst    d6fs_reader_commit_cache
d6fs_reader_write_block_bad:
        jrst    pdp10_ret_neg1

        .globl  d6fs_reader_zero_block
; int d6fs_reader_zero_block(reader, logical)
d6fs_reader_zero_block:
        jumpe   1,d6fs_reader_zero_block_bad
        caml    2,6(1)
        jrst    d6fs_reader_zero_block_bad
        pushj   17,fs_zero_block_workspace
        jrst    d6fs_reader_commit_cache
d6fs_reader_zero_block_bad:
        jrst    pdp10_ret_neg1

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
        lsh     4,-014
        andi    4,037
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

        .globl  d6fs_provider_blocks_for_words
; kword_t d6fs_provider_blocks_for_words(words)
d6fs_provider_blocks_for_words:
        jumpe   1,d6fs_provider_blocks_done
        addi    1,0177
        lsh     1,-7                    ; D6FS block = 128 words
d6fs_provider_blocks_done:
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
        movei   1,d6fs_provider_reader
        setz    3,
        pushj   17,d6fs_freemap_set
        jumpn   1,d6fs_provider_free_tail_done
        aoja    011,d6fs_provider_free_tail_loop
d6fs_provider_free_tail_ok:
        setz    1,
d6fs_provider_free_tail_done:
        pop     17,011
        pop     17,010
        popj    17,

        .globl  d6fs_provider_tail
; unsigned int d6fs_provider_tail(type, words, size_chars)
d6fs_provider_tail:
        jumpe   2,d6fs_provider_tail_zero
        caie    1,3                     ; D6FS_TYPE_SYMLINK
        jrst    d6fs_provider_tail_four
        subi    2,1
        move    5,2
        lsh     2,2
        lsh     5,1
        add     2,5                     ; base = (words - 1) * 6
        movei   4,6
        jrst    d6fs_provider_tail_check
d6fs_provider_tail_four:
        subi    2,1
        lsh     2,2                     ; base = (words - 1) * 4
        movei   4,4
d6fs_provider_tail_check:
        camg    3,2
        jrst    d6fs_provider_tail_full
        move    5,2
        add     5,4
        camle   3,5
        jrst    d6fs_provider_tail_full
        sub     3,2
        move    1,3
        popj    17,
d6fs_provider_tail_full:
        move    1,4
        popj    17,
d6fs_provider_tail_zero:
        jrst    pdp10_ret_zero

; Private provider helpers use the same 0/-1 convention as reader helpers.
; Keeping these in assembly avoids compiler result normalization and allows
; successful FCB validation to tail-call d6fs_reader_fcb directly.
        .globl  d6fs_provider_fcb
d6fs_provider_fcb:
        move    6,2
        move    7,3
        ldb     4,[POINT 6,1,11]
        jumpe   4,pdp10_ret_neg1
        move    5,d6fs_provider_reader+1
        andi    5,077
        came    4,5
        jrst    pdp10_ret_neg1
        hrrz    2,1
        movei   1,d6fs_provider_reader
        move    3,6
        move    4,7
        jrst    d6fs_reader_fcb

; unsigned int d6fs_provider_vtype(unsigned int type)
; D6FS types reaching this helper are validated REG/DIR/SYMLINK values 1..3.
; XCT keeps the three-value translation smaller than a branch chain/table.
        .globl  d6fs_provider_vtype
d6fs_provider_vtype:
        xct     d6fs_provider_vtype_ops-1(1)
        popj    17,
d6fs_provider_vtype_ops:
        movei   1,2                     ; REG -> VFS_TYPE_REG
        movei   1,1                     ; DIR -> VFS_TYPE_DIR
        movei   1,6                     ; SYMLINK -> VFS_TYPE_SYMLINK

; int d6fs_provider_sync(vnode_t node)
        .globl  d6fs_provider_sync
d6fs_provider_sync:
        ldb     1,[POINT 6,1,11]
        jumpe   1,pdp10_ret_neg1
        move    2,d6fs_provider_reader+1
        andi    2,077
        came    1,2
        jrst    pdp10_ret_neg1
        jrst    pdp10_ret_zero
