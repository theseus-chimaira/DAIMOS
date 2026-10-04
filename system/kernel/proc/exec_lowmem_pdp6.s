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
        .equ    PROC_FDCTL_OFFSET,045
        .equ    PROC_SWAP_BACKING_OFFSET,0427
        .equ    EXEC_LOAD_RT_REQUIRED,1

        ; Frame offsets relative to the fully advanced AC17.
        .equ    EXEC_LM_SAVE_FIRST,-010
        .equ    EXEC_LM_SAVE_LAST,-002
        .equ    EXEC_LM_LAUNCH,-001
        .equ    EXEC_LM_FRAME,011

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

        ; Allocate a transient copy before invalidating the mapped user source.
        setzm   EXEC_LM_LAUNCH(17)
        movei   1,EXEC_LM_LAUNCH(17)
        push    17,1                    ; fifth mm_alloc argument
        move    1,11
        movei   2,MM_TYPE_KERNEL_DYNAMIC
        movei   3,EXEC_LAUNCH_MM_OWNER
        setz    4,                      ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        sub     17,[1,,1]
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

        ; Return entry PC and move the temporary stack word from result[5]
        ; into its public slot.  argc/argv/envp are already in result[2..4].
        hlrz    1,(15)
        movem   1,(13)
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
