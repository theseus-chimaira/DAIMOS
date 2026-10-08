/**
 * @file mm_pdp6.s
 * @brief Compact PDP-6 permanent contiguous-core allocator.
 *
 * This is the PDP-6 implementation of the policy documented by mm.c.  The
 * portable C source remains the reference implementation.  PDP-6 uses a
 * sorted table of two-word allocated extents and up to three managed arenas;
 * free memory is represented implicitly by gaps.  The assembly path removes
 * KCC save frames and structure-copy scaffolding without changing policy.
 */

        .text
        .globl  mm_extents
        .globl  mm_arenas
        .globl  mm_core_words
        .globl  mm_extent_count
        .globl  mm_arena_count
        .globl  mm_alloc
        .globl  mm_alloc_aligned
        .globl  mm_alloc_aligned_noreclaim
        .globl  mm_free
        .globl  mm_pin
        .globl  mm_unpin
        .globl  mm_is_pinned
        .globl  mm_compact
        .globl  mm_extent_insert

        .globl  vm_extent_move
        .globl  proc_swap_reclaim
        .globl  fs_memory_reclaim
        .globl  mach_pi_disable
        .globl  mach_pi_restore

        .equ    MM_MAX_EXTENTS,025
        .equ    MM_MAX_ARENAS,3
        .equ    MM_TYPE_PROCESS,1
        .equ    MM_ALLOC_LOW,0
        .equ    MM_ALLOC_HIGH,1
        .equ    MM_ERR_NOMEM,-1
        .equ    MM_ERR_FRAGMENTED,-2
        .equ    MM_ERR_DESCRIPTORS,-3
        .equ    MM_ERR_INVAL,-4
        .equ    MM_ERR_BUSY,-5
        .equ    MM_PIN_ONE,010000000
        .equ    MM_PIN_FIELD,03770000000
        .equ    VM_EXTENT_ALIGN_WORDS,02000
        .equ    PROC_NO_SLOT,0400

; Delete extent slot AC1.  Private leaf, AC2..AC7 scratch.
mm_delete:
        move    2,mm_extent_count
        subi    2,1
        caml    1,2
        jrst    mm_delete_done
        move    3,1
mm_delete_loop:
        move    4,3
        addi    4,1
        lsh     4,1
        move    5,mm_extents(4)
        move    6,mm_extents+1(4)
        move    4,3
        lsh     4,1
        movem   5,mm_extents(4)
        movem   6,mm_extents+1(4)
        aoj     3,
        camge   3,2
        jrst    mm_delete_loop
mm_delete_done:
        sos     mm_extent_count
        popj    17,

; int mm_extent_insert(int slot, const struct mm_extent *extent)
mm_extent_insert:
        move    7,mm_extent_count
        caige   7,MM_MAX_EXTENTS
        jrst    mm_extent_insert_space
        movni   1,3
        popj    17,
mm_extent_insert_space:
        move    6,2
        caml    1,7
        jrst    mm_extent_insert_copy
        move    5,7
mm_extent_insert_shift:
        move    4,5
        subi    4,1
        lsh     4,1
        move    2,mm_extents(4)
        move    3,mm_extents+1(4)
        move    4,5
        lsh     4,1
        movem   2,mm_extents(4)
        movem   3,mm_extents+1(4)
        soj     5,
        camle   5,1
        jrst    mm_extent_insert_shift
mm_extent_insert_copy:
        move    4,1
        lsh     4,1
        move    2,(6)
        move    3,1(6)
        movem   2,mm_extents(4)
        movem   3,mm_extents+1(4)
        aos     mm_extent_count
        setz    1,
        popj    17,

; AC1=base. Return AC1=matching slot or extent_count.
mm_find_base:
        move    2,1
        setz    1,
mm_find_base_loop:
        caml    1,mm_extent_count
        popj    17,
        move    3,1
        lsh     3,1
        hrrz    4,mm_extents(3)
        camn    4,2
        popj    17,
        aoja    1,mm_find_base_loop

; int mm_is_pinned(kword_t base)
mm_is_pinned:
        pushj   17,mm_find_base
        caml    1,mm_extent_count
        jrst    mm_is_pinned_no
        lsh     1,1
        move    2,mm_extents+1(1)
        tlne    2,03770
        jrst    mm_is_pinned_yes
mm_is_pinned_no:
        setz    1,
        popj    17,
mm_is_pinned_yes:
        movei   1,1
        popj    17,

; Find a fitting free gap.
; AC1=words, AC2=alignment, AC3=preference, AC4=basep.
; Return AC1=1 fit / 0 no fit. On failure *basep=total free words.
mm_find_fit:
        add     17,[010,,010]
        movei   0,-7(17)
        hrli    0,010
        blt     0,-1(17)
        move    010,1
        move    011,2
        move    012,3
        move    013,4
        setz    014,
        setz    015,
        seto    016,
        setzm   (17)

mm_find_fit_arena:
        caml    014,mm_arena_count
        jrst    mm_find_fit_finish
        move    1,014
        hrrz    4,mm_arenas(1)
        hlrz    5,mm_arenas(1)
        add     5,4

mm_find_fit_skip:
        caml    015,mm_extent_count
        jrst    mm_find_fit_tail
        move    1,015
        lsh     1,1
        hrrz    2,mm_extents(1)
        caml    2,4
        jrst    mm_find_fit_extent
        aoja    015,mm_find_fit_skip

mm_find_fit_extent:
        caml    015,mm_extent_count
        jrst    mm_find_fit_tail
        move    1,015
        lsh     1,1
        hrrz    2,mm_extents(1)
        caml    2,5
        jrst    mm_find_fit_tail
        camg    2,4
        jrst    mm_find_fit_after_gap
        move    6,2
        sub     6,4
        addm    6,(17)
        jumpe   012,mm_find_fit_low_gap
        camge   6,010
        jrst    mm_find_fit_after_gap
        move    7,2
        sub     7,010
        move    3,011
        subi    3,1
        setcm   3,3
        and     7,3
        caml    7,4
        move    016,7
        jrst    mm_find_fit_after_gap
mm_find_fit_low_gap:
        move    7,4
        add     7,011
        subi    7,1
        move    3,011
        subi    3,1
        setcm   3,3
        and     7,3
        move    6,2
        sub     6,7
        camg    010,6
        jrst    mm_find_fit_low_found

mm_find_fit_after_gap:
        move    1,015
        lsh     1,1
        hrrz    2,mm_extents(1)
        hlrz    3,mm_extents(1)
        add     3,2
        camle   3,4
        move    4,3
        aoja    015,mm_find_fit_extent

mm_find_fit_tail:
        camg    5,4
        jrst    mm_find_fit_next_arena
        move    6,5
        sub     6,4
        addm    6,(17)
        jumpe   012,mm_find_fit_low_tail
        camge   6,010
        jrst    mm_find_fit_next_arena
        move    7,5
        sub     7,010
        move    3,011
        subi    3,1
        setcm   3,3
        and     7,3
        caml    7,4
        move    016,7
        jrst    mm_find_fit_next_arena
mm_find_fit_low_tail:
        move    7,4
        add     7,011
        subi    7,1
        move    3,011
        subi    3,1
        setcm   3,3
        and     7,3
        move    6,5
        sub     6,7
        camg    010,6
        jrst    mm_find_fit_low_found

mm_find_fit_next_arena:
        aoja    014,mm_find_fit_arena

mm_find_fit_low_found:
        movem   7,(013)
        movei   1,1
        jrst    mm_find_fit_return
mm_find_fit_finish:
        jumpge  016,mm_find_fit_high_found
        move    1,(17)
        movem   1,(013)
        setz    1,
        jrst    mm_find_fit_return
mm_find_fit_high_found:
        movem   016,(013)
        movei   1,1
mm_find_fit_return:
        movei   0,010
        hrli    0,-7(17)
        blt     0,016
        sub     17,[010,,010]
        popj    17,

; int mm_alloc_aligned_noreclaim(words,align,type,owner,pref,basep)
mm_alloc_aligned_noreclaim:
        move    5,-1(17)
        move    6,-2(17)
        add     17,[011,,011]
        movei   0,-010(17)
        hrli    0,010
        blt     0,-2(17)
        move    010,1
        move    011,2
        move    012,3
        move    013,4
        move    014,5
        move    015,6
        jumpe   015,mm_alloc_nr_inval
        jumpe   010,mm_alloc_nr_inval
        tlne    010,0777777
        jrst    mm_alloc_nr_inval
        jumpe   011,mm_alloc_nr_inval
        tlne    011,0777777
        jrst    mm_alloc_nr_inval
        move    1,011
        subi    1,1
        and     1,011
        jumpn   1,mm_alloc_nr_inval
        jumpe   012,mm_alloc_nr_inval
        move    1,012
        and     1,[-4]
        jumpn   1,mm_alloc_nr_inval
        move    1,014
        and     1,[-2]
        jumpn   1,mm_alloc_nr_inval

        movei   4,-1(17)
        move    1,010
        move    2,011
        move    3,014
        pushj   17,mm_find_fit
        jumpn   1,mm_alloc_nr_fit
        move    1,-1(17)
        camge   1,010
        jrst    mm_alloc_nr_nomem
        movni   1,2
        jrst    mm_alloc_nr_return
mm_alloc_nr_nomem:
        seto    1,
        jrst    mm_alloc_nr_return

mm_alloc_nr_fit:
        move    016,-1(17)
        setz    1,
mm_alloc_nr_slot:
        caml    1,mm_extent_count
        jrst    mm_alloc_nr_build
        move    2,1
        lsh     2,1
        hrrz    3,mm_extents(2)
        caml    3,016
        jrst    mm_alloc_nr_build
        aoja    1,mm_alloc_nr_slot
mm_alloc_nr_build:
        hrlz    2,010
        hrr     2,016
        movem   2,-1(17)
        move    2,012
        lsh     2,022
        hrr     2,013
        movem   2,(17)
        movei   2,-1(17)
        pushj   17,mm_extent_insert
        jumpn   1,mm_alloc_nr_descriptors
        movem   016,(015)
        setz    1,
        jrst    mm_alloc_nr_return
mm_alloc_nr_descriptors:
        movni   1,3
        jrst    mm_alloc_nr_return
mm_alloc_nr_inval:
        movni   1,4
mm_alloc_nr_return:
        movei   0,010
        hrli    0,-010(17)
        blt     0,016
        sub     17,[011,,011]
        popj    17,

; Move one process extent upward. AC1=slot, AC2=alignment.
mm_move_extent:
        add     17,[015,,015]
        movei   0,-014(17)
        hrli    0,010
        blt     0,-6(17)
        move    010,1
        move    011,2
        move    1,010
        lsh     1,1
        move    2,mm_extents(1)
        move    3,mm_extents+1(1)
        movem   2,-5(17)
        movem   3,-4(17)
        hrrz    012,2
        hlrz    013,2
        pushj   17,mach_pi_disable
        movem   1,-3(17)
        move    1,010
        pushj   17,mm_delete
        move    1,013
        move    2,011
        movei   3,MM_ALLOC_HIGH
        movei   4,-2(17)
        pushj   17,mm_find_fit
        movem   1,(17)
        move    1,010
        movei   2,-5(17)
        pushj   17,mm_extent_insert
        skipn   (17)
        jrst    mm_move_fragmented
        move    015,-2(17)
        camg    015,012
        jrst    mm_move_fragmented

        hrrz    1,-4(17)
        move    2,012
        move    3,013
        move    4,015
        pushj   17,vm_extent_move
        movem   1,-1(17)
        jumpn   1,mm_move_restore_pi

        hrlz    1,013
        hrr     1,015
        movem   1,-5(17)
        move    1,010
        pushj   17,mm_delete
        setz    014,
mm_move_new_slot:
        caml    014,mm_extent_count
        jrst    mm_move_publish
        move    1,014
        lsh     1,1
        hrrz    2,mm_extents(1)
        caml    2,015
        jrst    mm_move_publish
        aoja    014,mm_move_new_slot
mm_move_publish:
        move    1,014
        movei   2,-5(17)
        pushj   17,mm_extent_insert
        jrst    mm_move_restore_pi

mm_move_fragmented:
        movni   1,2
        movem   1,-1(17)
mm_move_restore_pi:
        move    1,-3(17)
        pushj   17,mach_pi_restore
        move    1,-1(17)
        movei   0,010
        hrli    0,-014(17)
        blt     0,016
        sub     17,[015,,015]
        popj    17,

; int mm_compact(words, alignment)
mm_compact:
        add     17,[011,,011]
        movei   0,-010(17)
        hrli    0,010
        blt     0,-2(17)
        move    010,1
        move    011,2
        jumpe   010,mm_compact_inval
        tlne    010,0777777
        jrst    mm_compact_inval
        jumpe   011,mm_compact_inval
        tlne    011,0777777
        jrst    mm_compact_inval
        move    1,011
        subi    1,1
        and     1,011
        jumpn   1,mm_compact_inval
        move    1,010
        move    2,011
        movei   3,MM_ALLOC_LOW
        movei   4,(17)
        pushj   17,mm_find_fit
        jumpn   1,mm_compact_ok
        move    1,(17)
        camge   1,010
        jrst    mm_compact_nomem
        setz    012,

mm_compact_scan:
        caml    012,mm_extent_count
        jrst    mm_compact_fragmented
        move    1,012
        lsh     1,1
        move    2,mm_extents+1(1)
        hlrz    3,2
        andi    3,7
        caie    3,MM_TYPE_PROCESS
        jrst    mm_compact_next
        tlne    2,03770
        jrst    mm_compact_next
        move    1,012
        movei   2,VM_EXTENT_ALIGN_WORDS
        pushj   17,mm_move_extent
        jumpn   1,mm_compact_next
        move    1,010
        move    2,011
        movei   3,MM_ALLOC_LOW
        movei   4,(17)
        pushj   17,mm_find_fit
        jumpn   1,mm_compact_ok
        setz    012,
        jrst    mm_compact_scan
mm_compact_next:
        aoja    012,mm_compact_scan
mm_compact_ok:
        setz    1,
        jrst    mm_compact_return
mm_compact_nomem:
        seto    1,
        jrst    mm_compact_return
mm_compact_fragmented:
        movni   1,2
        jrst    mm_compact_return
mm_compact_inval:
        movni   1,4
mm_compact_return:
        movei   0,010
        hrli    0,-010(17)
        blt     0,016
        sub     17,[011,,011]
        popj    17,

; int mm_alloc(words,type,owner,preference,basep)
mm_alloc:
        move    5,-1(17)
        push    17,5
        push    17,4
        move    4,3
        move    3,2
        movei   2,1
        pushj   17,mm_alloc_aligned
        sub     17,[2,,2]
        popj    17,

; int mm_alloc_aligned(words,align,type,owner,preference,basep)
mm_alloc_aligned:
        move    5,-1(17)
        move    6,-2(17)
        add     17,[011,,011]
        movei   0,-010(17)
        hrli    0,010
        blt     0,-2(17)
        move    010,1
        move    011,2
        move    012,3
        move    013,4
        move    014,5
        move    015,6
        setz    016,
mm_alloc_aligned_retry:
        push    17,015
        push    17,014
        move    1,010
        move    2,011
        move    3,012
        move    4,013
        pushj   17,mm_alloc_aligned_noreclaim
        sub     17,[2,,2]
        movem   1,(17)
        jumpe   016,mm_alloc_aligned_after_compact
        came    1,[MM_ERR_FRAGMENTED]  ; negative status needs full-word compare
        jrst    mm_alloc_aligned_after_compact
        move    1,010
        move    2,011
        pushj   17,mm_compact
        jumpn   1,mm_alloc_aligned_store_compact
        push    17,015
        push    17,014
        move    1,010
        move    2,011
        move    3,012
        move    4,013
        pushj   17,mm_alloc_aligned_noreclaim
        sub     17,[2,,2]
mm_alloc_aligned_store_compact:
        movem   1,(17)

mm_alloc_aligned_after_compact:
        move    1,(17)
        jumpe   1,mm_alloc_aligned_return
        camge   1,[-3]
        jrst    mm_alloc_aligned_return
        caige   016,3
        jrst    mm_alloc_aligned_pressure
        jrst    mm_alloc_aligned_return

mm_alloc_aligned_pressure:
        caige   016,2
        jrst    mm_alloc_aligned_fs_reclaim
        movei   3,PROC_NO_SLOT
        caie    012,MM_TYPE_PROCESS
        jrst    mm_alloc_aligned_swap
        move    3,013
mm_alloc_aligned_swap:
        move    1,010
        move    2,011
        pushj   17,proc_swap_reclaim
        jrst    mm_alloc_aligned_pressure_done
mm_alloc_aligned_fs_reclaim:
        move    1,010
        move    2,016
        pushj   17,fs_memory_reclaim
mm_alloc_aligned_pressure_done:
        aoj     016,
        jrst    mm_alloc_aligned_retry

mm_alloc_aligned_return:
        movei   0,010
        hrli    0,-010(17)
        blt     0,016
        sub     17,[011,,011]
        popj    17,

; int mm_free(base,type,owner)
mm_free:
        move    5,1
        move    6,2
        move    7,3
        pushj   17,mm_find_base
        caml    1,mm_extent_count
        jrst    mm_free_inval
        move    4,1
        lsh     4,1
        move    2,mm_extents+1(4)
        hlrz    3,2
        andi    3,7
        came    3,6
        jrst    mm_free_inval
        hrrz    3,2
        came    3,7
        jrst    mm_free_inval
        tlne    2,03770
        jrst    mm_free_busy
        pushj   17,mm_delete
        setz    1,
        popj    17,
mm_free_inval:
        movni   1,4
        popj    17,
mm_free_busy:
        movni   1,5
        popj    17,

; AC1=base, AC2=+1/-1. Shared pin adjustment.
mm_pin_adjust:
        move    6,2
        pushj   17,mm_find_base
        caml    1,mm_extent_count
        jrst    mm_pin_inval
        lsh     1,1
        move    4,mm_extents+1(1)
        jumpg   6,mm_pin_add
        tlne    4,03770
        jrst    mm_pin_sub
mm_pin_inval:
        movni   1,4
        popj    17,
mm_pin_add:
        move    5,4
        and     5,[MM_PIN_FIELD]
        came    5,[MM_PIN_FIELD]
        jrst    mm_pin_add_ok
        movni   1,5
        popj    17,
mm_pin_add_ok:
        add     4,[MM_PIN_ONE]
        movem   4,mm_extents+1(1)
        setz    1,
        popj    17,
mm_pin_sub:
        sub     4,[MM_PIN_ONE]
        movem   4,mm_extents+1(1)
        setz    1,
        popj    17,

mm_pin:
        movei   2,1
        jrst    mm_pin_adjust

mm_unpin:
        seto    2,
        jrst    mm_pin_adjust

        .bss
mm_extents:
        .block  052
mm_arenas:
        .block  MM_MAX_ARENAS
mm_core_words:
        .block  1
mm_extent_count:
        .block  1
mm_arena_count:
        .block  1
