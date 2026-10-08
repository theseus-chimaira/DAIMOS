/**
 * @file exec_lowmem_pdp6.s
 * @brief Compact destructive low-memory EXEC retry for the PDP-6.
 *
 * The ordinary EXEC path remains transactional: it loads a staged VM before
 * destroying the current one.  A 32K machine may not have enough free core
 * for both images.  exec_load_process() reports EXEC_LOAD_NOMEM before it has
 * attached swap backing and after rolling back any new RT reservation.  This
 * helper then copies the already-validated bounded EXEC launch block into
 * kernel-dynamic core, releases the old image, and retries from that copy.
 *
 * C ABI:
 *   AC1 = mapped EXEC launch block
 *   AC2 = launch-block words
 *   AC3 = argc,,envc
 *   AC4 = result scratch with six writable words; words 0..4 are the public
 *         entry, stack, argc, argv, envp result and word 5 is temporary
 *   AC1 return = 0 success, -1 before destructive commit, -2 after it
 *
 * AC10..AC16 are preserved.  The local frame holds seven saved ACs plus one
 * launch-copy base word.  After vm_space_destroy() succeeds the old image is
 * already irrecoverable, so the retry loads directly into the current process
 * descriptor instead of maintaining a staged three-word proc copy.
 */

        .text
        .globl  exec_replace_current_lowmem
        .globl  mm_alloc
        .globl  mm_free
        .globl  vm_space_destroy
        .globl  vm_space_startup
        .globl  vm_user_mapping_hold
        .globl  vm_user_mapping_release
        .globl  exec_load_process
        .globl  sixbit_record_words
        .globl  proc_current_slot
        .globl  proc_slot_ptr
        .globl  proc_rt_owner
        .globl  fs_copy_words

        .equ    EXEC_LAUNCH_MM_OWNER,013
        .equ    MM_TYPE_KERNEL_DYNAMIC,3
        .equ    PROC_USER_MAP_BIT,2
        .equ    PROC_FDCTL_OFFSET,024
        .equ    PROC_SWAP_BACKING_OFFSET,0406
        .equ    PROC_BRK_OFFSET,0407
        .equ    EXEC_LOAD_RT_REQUIRED,1
        .equ    EXEC_LOAD_NOMEM,-2

        .equ    EXEC_R_SAVE_FIRST,-020
        .equ    EXEC_R_SAVE_LAST,-012
        .equ    EXEC_R_STAGED,-011
        .equ    EXEC_R_STARTUP,-006
        .equ    EXEC_R_OLD_SWAP,-002
        .equ    EXEC_R_NEW_SWAP,-001
        .equ    EXEC_R_COUNTS,0
        .equ    EXEC_R_FRAME,021

/**
 * @brief Transactionally replace the current process image.
 *
 * @param AC1 Mapped EXEC V1 launch block.
 * @param AC2 Available mapped words.
 * @param AC3 Five-word result vector: entry, stack, argc, argv, envp.
 * @return AC1 zero on success or -1 before commit failure.
 *
 * The C transaction is intentionally preserved exactly: validate the complete
 * bounded launch block, stage a replacement VM, construct startup records,
 * then destroy the old image only after the replacement is complete.  The
 * compact PDP-6 spelling keeps validation pointers and transaction identity in
 * AC10..AC16 and uses only ten local words in addition to the seven saved ACs.
 */
        .globl  exec_replace_current
        .globl  proc_current_ptr
        .globl  proc_swap_records
exec_replace_current:
        add     17,[EXEC_R_FRAME,,EXEC_R_FRAME]
        movei   0,EXEC_R_SAVE_FIRST(17)
        hrli    0,010
        blt     0,EXEC_R_SAVE_LAST(17)
        move    10,1                   ; launch block
        move    11,2                   ; available words, then launch words
        move    12,3                   ; result vector

        caige   11,5
        jrst    exec_replace_invalid
        hlrz    1,(10)
        caie    1,1                    ; SYS_EXEC_VERSION_1
        jrst    exec_replace_invalid
        hrrz    7,(10)
        caige   7,5
        jrst    exec_replace_invalid
        move    1,11
        sub     1,7
        jumpl   1,exec_replace_invalid
        move    11,7

        move    1,1(10)                ; argc must be a pure RH value
        tlne    1,0777777
        jrst    exec_replace_invalid
        hrrz    13,1
        caile   13,020
        jrst    exec_replace_invalid
        move    1,2(10)                ; envc
        tlne    1,0777777
        jrst    exec_replace_invalid
        hrrz    14,1
        caile   14,020
        jrst    exec_replace_invalid
        hrlz    1,13
        ior     1,14
        movem   1,EXEC_R_COUNTS(17)

        move    16,10
        add     16,11                  ; exact end of launch block
        movei   15,3(10)               ; counted path record
        move    1,15
        movei   2,1
        pushj   17,sixbit_record_words
        jumpe   1,exec_replace_invalid
        move    2,15
        add     2,1
        move    3,16
        sub     3,2
        jumpl   3,exec_replace_invalid
        move    15,2                   ; argv/env record stream

        add     13,14                  ; number of following records
exec_replace_validate_records:
        jumpe   13,exec_replace_validate_done
        move    1,16
        sub     1,15
        jumple  1,exec_replace_invalid
        move    1,15
        setz    2,
        pushj   17,sixbit_record_words
        jumpe   1,exec_replace_invalid
        add     15,1
        move    2,16
        sub     2,15
        jumpl   2,exec_replace_invalid
        sojg    13,exec_replace_validate_records
exec_replace_validate_done:
        came    15,16
        jrst    exec_replace_invalid

        ; Reconstruct the records pointer from the validated path span because
        ; AC15 now equals END after the validation walk.
        movei   15,3(10)
        move    1,15
        movei   2,1
        pushj   17,sixbit_record_words
        add     15,1

        move    13,proc_current_slot
        move    16,proc_current_ptr
        setz    14,
        move    1,proc_rt_owner
        came    1,13
        jrst    exec_replace_old_rt_done
        movei   14,1
exec_replace_old_rt_done:
        move    1,proc_swap_records
        add     1,13
        move    2,(1)
        movem   2,EXEC_R_OLD_SWAP(17)

        move    1,(16)
        movem   1,EXEC_R_STAGED(17)     ; staged.meta = current->meta
        movei   1,EXEC_R_STAGED(17)
        move    2,13
        movei   3,3(10)                 ; validated path
        pushj   17,exec_load_process
        movem   1,EXEC_R_STAGED+2(17)   ; staged.sched is dead; keep result here
        came    1,[EXEC_LOAD_NOMEM]    ; negative status needs full-word compare
        jrst    exec_replace_loaded
        move    1,10
        move    2,11
        move    3,EXEC_R_COUNTS(17)
        move    4,12
        pushj   17,exec_replace_current_lowmem
        jrst    exec_replace_return

exec_replace_loaded:
        jumpl   1,exec_replace_restore_fail
        movei   1,EXEC_R_STAGED(17)
        move    2,15
        move    3,EXEC_R_COUNTS(17)
        movei   4,EXEC_R_STARTUP(17)
        pushj   17,vm_space_startup
        jumpe   1,exec_replace_startup_ok

        movei   1,EXEC_R_STAGED(17)
        move    2,13
        pushj   17,vm_space_destroy
        jrst    exec_replace_restore_fail

exec_replace_startup_ok:
        move    1,proc_swap_records
        add     1,13
        move    2,(1)
        movem   2,EXEC_R_NEW_SWAP(17)
        move    2,EXEC_R_OLD_SWAP(17)
        movem   2,(1)

        ; The launch block is in the old VM.  Clear its mapping hold before
        ; destroying that VM; restore the hold if destruction fails.
        hlrz    1,(16)
        move    2,PROC_FDCTL_OFFSET(1)
        andcmi  2,PROC_USER_MAP_BIT
        movem   2,PROC_FDCTL_OFFSET(1)
        move    1,16
        move    2,13
        pushj   17,vm_space_destroy
        jumpe   1,exec_replace_commit

        hlrz    1,(16)
        move    2,PROC_FDCTL_OFFSET(1)
        iori    2,PROC_USER_MAP_BIT
        movem   2,PROC_FDCTL_OFFSET(1)
        movei   1,EXEC_R_STAGED(17)
        move    2,13
        pushj   17,vm_space_destroy
        jrst    exec_replace_restore_fail

exec_replace_commit:
        move    1,EXEC_R_STAGED+1(17)
        movem   1,1(16)
        ; Match the portable EXEC commit: the stable u-area survives image
        ; replacement, so reset its heap floor/current break to the complete
        ; newly committed VM rather than inheriting the old program's break.
        hlrz    1,(16)
        hlrz    2,1(16)
        hrl     2,2
        movem   2,PROC_BRK_OFFSET(1)
        setzm   PROC_SWAP_BACKING_OFFSET(1)
        move    1,proc_swap_records
        add     1,13
        move    2,EXEC_R_NEW_SWAP(17)
        movem   2,(1)

        hlrz    1,EXEC_R_STAGED(17)
        movem   1,(12)
        move    1,EXEC_R_STARTUP+3(17)
        movem   1,1(12)
        move    1,EXEC_R_STARTUP(17)
        movem   1,2(12)
        move    1,EXEC_R_STARTUP+1(17)
        movem   1,3(12)
        move    1,EXEC_R_STARTUP+2(17)
        movem   1,4(12)

        jumpe   14,exec_replace_success
        skipn   EXEC_R_STAGED+2(17)     ; old RT + new non-RT image
        jrst    exec_replace_release_rt
        jrst    exec_replace_success
exec_replace_release_rt:
        move    1,proc_rt_owner
        came    1,13
        jrst    exec_replace_success
        setzm   proc_rt_owner
exec_replace_success:
        setz    1,
        jrst    exec_replace_return

exec_replace_restore_fail:
        ; exec_load_process normally rolls back a newly acquired RT owner on
        ; its own failure.  Startup/commit failures occur after a successful
        ; load and therefore need the C transaction's explicit rollback.
        jumpn   14,exec_replace_restore_swap
        move    1,EXEC_R_STAGED+2(17)
        caie    1,EXEC_LOAD_RT_REQUIRED
        jrst    exec_replace_restore_swap
        move    1,proc_rt_owner
        came    1,13
        jrst    exec_replace_restore_swap
        setzm   proc_rt_owner
exec_replace_restore_swap:
        move    1,proc_swap_records
        add     1,13
        move    2,EXEC_R_OLD_SWAP(17)
        movem   2,(1)
exec_replace_invalid:
        seto    1,
exec_replace_return:
        movei   0,010
        hrli    0,EXEC_R_SAVE_FIRST(17)
        blt     0,016
        sub     17,[EXEC_R_FRAME,,EXEC_R_FRAME]
        popj    17,

        ; Frame offsets relative to the fully advanced AC17.
        .equ    EXEC_LM_SAVE_FIRST,-010
        .equ    EXEC_LM_SAVE_LAST,-002
        .equ    EXEC_LM_META,-002
        .equ    EXEC_LM_LAUNCH,-001
        .equ    EXEC_LM_FRAME,012

exec_replace_current_lowmem:
        add     17,[EXEC_LM_FRAME,,EXEC_LM_FRAME]
        movei   0,EXEC_LM_SAVE_FIRST(17)
        hrli    0,010
        blt     0,EXEC_LM_SAVE_LAST(17)

        move    10,1                    ; original mapped launch block
        move    11,2                    ; launch words
        move    12,3                    ; argc,,envc
        move    13,4                    ; caller result vector
        move    14,proc_current_slot

        ; Descriptor lookup is cold here; reuse the resident slot helper.
        move    1,14
        pushj   17,proc_slot_ptr
        move    15,1
        move    1,(15)
        movem   1,EXEC_LM_META(17)      ; stable u-area/pgrp metadata

        ; Allocate a transient copy before invalidating the mapped user source.
        setzm   EXEC_LM_LAUNCH(17)
        movei   1,EXEC_LM_LAUNCH(17)
        push    17,1                    ; fifth mm_alloc argument
        move    1,11
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,EXEC_LAUNCH_MM_OWNER
        setz    4,                      ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        sub     17,kconst_1_1
        jumpn   1,exec_lowmem_alloc_fail

        ; The source and destination are distinct MM extents.  Reuse the
        ; resident overlap-safe word copier rather than open-coding BLT setup.
        move    1,10
        move    2,EXEC_LM_LAUNCH(17)
        move    3,11
        pushj   17,fs_copy_words

        ; Release the direct user-map hold before destroying its containing VM.
        pushj   17,vm_user_mapping_release

        move    1,15
        move    2,14
        pushj   17,vm_space_destroy
        jumpe   1,exec_lowmem_old_gone

        ; Destruction failed before proc_swap_detach; the old image remains
        ; valid, so restore its user-map hold and return non-destructively.
        pushj   17,vm_user_mapping_hold
        seto    16,
        jrst    exec_lowmem_free

exec_lowmem_old_gone:
        move    10,EXEC_LM_LAUNCH(17)
        addi    10,3                    ; copied SYS_EXEC_V1 path record
        move    1,10
        movei   2,1
        pushj   17,sixbit_record_words
        add     1,10
        move    11,1                    ; argv/env record stream

        move    1,15
        move    2,14
        move    3,10
        pushj   17,exec_load_process
        move    16,1
        jumpl   1,exec_lowmem_fatal

        ; exec_load_process normally populates a detached staged descriptor
        ; and therefore keeps only parent+entry in META.  Low-memory EXEC must
        ; load directly into the live descriptor after freeing the old VM, so
        ; capture the new entry and restore the stable process metadata before
        ; any u-area-dependent startup/credential path observes the descriptor.
        hlrz    1,(15)
        movem   1,(13)
        move    1,EXEC_LM_META(17)
        movem   1,(15)

        move    1,15
        move    2,11
        move    3,12
        movei   4,2(13)                 ; argc,argv,envp,stack -> result[2..5]
        pushj   17,vm_space_startup
        jumpe   1,exec_lowmem_commit

        ; The replacement VM exists but startup construction failed.  Release
        ; it before returning the fatal-after-commit indication.
        move    1,15
        move    2,14
        pushj   17,vm_space_destroy
        jrst    exec_lowmem_fatal

exec_lowmem_commit:
        ; exec_load_process populated the current descriptor directly and
        ; attached the replacement executable backing in proc_swap_records.
        ; The old u-area survives VM replacement; no swapped-image descriptor
        ; may remain there after the replacement becomes resident.
        hlrz    1,(15)
        setzm   PROC_SWAP_BACKING_OFFSET(1)
        ; Reset floor,,current break to the complete newly allocated VM.  The
        ; heap therefore begins above the fixed startup/stack reservation.
        hlrz    2,1(15)
        hrl     2,2
        movem   2,PROC_BRK_OFFSET(1)

        ; Return entry PC and move the temporary stack word from result[5]
        ; into its public slot.  argc/argv/envp are already in result[2..4].
        move    1,5(13)
        movem   1,1(13)

        ; A successful non-RT replacement releases prior ownership.  An
        ; RT_REQUIRED replacement leaves the slot as the active RT owner.
        cain    16,EXEC_LOAD_RT_REQUIRED
        jrst    exec_lowmem_success
exec_lowmem_release_old_rt:
        move    1,proc_rt_owner
        camn    1,14
        setzm   proc_rt_owner
exec_lowmem_success:
        setz    16,
        jrst    exec_lowmem_free

exec_lowmem_fatal:
        movni   16,2                    ; EXEC_REPLACE_FATAL

exec_lowmem_free:
        move    1,EXEC_LM_LAUNCH(17)
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,EXEC_LAUNCH_MM_OWNER
        pushj   17,mm_free
        move    1,16
        jrst    exec_lowmem_return

exec_lowmem_alloc_fail:
        seto    1,
exec_lowmem_return:
        movei   0,010
        hrli    0,EXEC_LM_SAVE_FIRST(17)
        blt     0,016
        sub     17,[EXEC_LM_FRAME,,EXEC_LM_FRAME]
        popj    17,
