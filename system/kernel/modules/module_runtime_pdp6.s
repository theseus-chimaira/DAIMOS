/**
 * @file module_runtime_pdp6.s
 * @brief Compact PDP-6 runtime movable-module relocation primitive.
 *
 * This is the target implementation of the deferred module_runtime_move() ABI.
 * It copies image+retained relocation map, applies two-bit LH/RH relocations,
 * retargets fixed/dynamic bindings, then publishes the new base.
 * The future MM caller must validate the move and keep PI disabled throughout.
 * The code uses only PDP-6-compatible instructions; later PDP-10 models may
 * eventually gain separate optimized implementations. It is not linked into
 * today's KCORE because runtime movable modules remain a post-overlay feature.
 */

        .text
        .globl  module_runtime_move
        .extern fs_move_words
        .extern module_runtime_descs
        .extern module_dynamic_bindings

        /*
         * Fixed KCORE words whose RH may point into a movable module.
         *
         * Keep this table beside the PDP-6 mover so its loop count is derived
         * from the labels instead of duplicated as a hand-maintained literal.
         * Adding a binding therefore cannot silently leave the tail unretargeted.
         */
        .data
module_fixed_bindings:
        .word   pdp10_pi_level1_dispatch_jump
        .word   pdp10_pi_level2_dispatch_jump
        .word   pdp10_pi_level3_dispatch_jump
        .word   pdp10_pi_level4_dispatch_jump
        .word   pdp10_pi_level5_dispatch_jump
        .word   pdp10_pi_level6_dispatch_jump
        .word   native_sys_putchar_call
        .word   native_sys_getchar_call
        .word   tty_write_s6rec_jump
        .word   tty_read_s6rec_jump
        .word   ptr_read_words_jump
        .word   ptp_write_words_jump
        .word   cr_read_words_jump
        .word   cp_write_words_jump
        .word   lpt_putchar_jump
        .word   lpt_write_s6rec_jump
        .word   dpy_write_words_jump
        .word   wcnsls_read_words_jump
        .word   wcnsls_write_words_jump
        .word   sys_dtc_read_block_jump
        .word   sys_dtc_write_block_jump
        .word   storage_pi_dsk_jump
        .word   storage_dct_dsk_jump
        .word   storage_pi_tape_jump
        .word   storage_dct_tape_jump
        .word   storage_clock_dsk_jump
        .word   dsk270_read_jump
        .word   dsk270_write_jump
        .word   drm236_read_jump
        .word   drm236_write_jump
        .word   fs_memfs_service_jump
        .word   sys_memfs_usage_call
        .word   fs_dtfs_service_jump
        .word   sys_dtfs_mount_jump
        .word   fs_d6fs_service_jump
        .word   fs_tsfs_service_jump
        .word   memfs_reclaim_jump
        .word   memfs_shutdown_jump
        .word   blockset_runtime_service_jump
module_fixed_bindings_end:

        .text

; Persistent registers while the move is in progress:
; 10 owner, 11 new base, 12 total words, 13 old base,
; 14 image words, 15 initialized words, 16 saved descriptor.
module_runtime_move:
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,10
        blt     0,(17)
        move    10,1
        move    11,2
        move    12,3
        move    16,module_runtime_descs(10)
        hrrz    13,16
        hlrz    15,16

        ; image_words = total_words - ((init_words + 17) / 18).
        move    5,15
        addi    5,021
        idivi   5,022
        move    14,12
        sub     14,5

        ; MM owns range/state validation and holds PI disabled.
        move    1,13
        move    2,11
        move    3,12
        pushj   17,fs_move_words

        ; Relocate initialized words from the sequential two-bit map.
        move    6,11
        add     6,14
        move    7,0(6)
        movei   5,022
        move    1,11
        move    2,15
        jumpe   2,module_runtime_reloc_done
module_runtime_reloc_loop:
        move    3,7
        lsh     3,-042
        lsh     7,2
        jumpe   3,module_runtime_reloc_next
        move    4,0(1)
        trnn    3,2
        jrst    module_runtime_reloc_rh
        hlrz    0,4
        sub     0,13
        add     0,11
        hrlm    0,4
module_runtime_reloc_rh:
        trnn    3,1
        jrst    module_runtime_reloc_store
        hrrz    0,4
        sub     0,13
        add     0,11
        hrrm    0,4
module_runtime_reloc_store:
        movem   4,0(1)
module_runtime_reloc_next:
        addi    1,1
        subi    2,1
        jumpe   2,module_runtime_reloc_done
        sojg    5,module_runtime_reloc_loop
        addi    6,1
        move    7,0(6)
        movei   5,022
        jrst    module_runtime_reloc_loop

module_runtime_reloc_done:
        movei   5,module_fixed_bindings
        movei   6,module_fixed_bindings_end-module_fixed_bindings
module_runtime_fixed_loop:
        move    1,0(5)
        pushj   17,module_runtime_retarget_asm
        addi    5,1
        sojg    6,module_runtime_fixed_loop

        ; Dynamic binding slots can themselves live in the moved module.
        movei   5,module_dynamic_bindings
        movei   6,010
module_runtime_dynamic_loop:
        move    2,0(5)
        jumpe   2,module_runtime_dynamic_done
        hlrz    3,2
        hrrz    4,2
        move    1,11
        came    3,10
        hrrz    1,module_runtime_descs(3)
        add     1,4
        pushj   17,module_runtime_retarget_asm
        addi    5,1
        sojg    6,module_runtime_dynamic_loop
module_runtime_dynamic_done:
        hrrm    11,16
        movem   16,module_runtime_descs(10)
        setz    1,
        movei   0,10
        hrli    0,-6(17)
        blt     0,16
        sub     17,[7,,7]
        popj    17,

; AC1 = address of a binding word.  Retarget its RH if it points into the
; old initialized image.  Persistent AC10-16 remain unchanged.
module_runtime_retarget_asm:
        hrrz    2,0(1)
        camge   2,13
        popj    17,
        sub     2,13
        caml    2,14
        popj    17,
        move    3,0(1)
        move    4,11
        add     4,2
        hrrm    4,3
        movem   3,0(1)
        popj    17,
