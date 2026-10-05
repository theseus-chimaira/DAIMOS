; proc_run.s -- compact resident RUN construction for PDP-6.
;
; RUN is implemented here rather than keeping the boot-time C constructor
; resident.  The three-word process descriptor remains the permanent table
; cost.  The child u-area contains explicit descriptor inheritance state.
;
; C ABI entry:
;   AC1 = physical address of struct sys_run_v2
;   AC2 = user words available from AC1 through end of user extent
; Return AC1 = child PID/slot, or -1.

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000
        .equ    PROC_SCHED_SIDL_LH,0100024
        .equ    PROC_SCHED_SRUN_LH,0200024
        .equ    PROC_F_UAREA_RH,0400000
        .equ    PROC_UAREA_WORDS,0407
        .equ    PROC_FDCTL_OFFSET,024
        .equ    PROC_FILE_CWD_OFFSET,025
        .equ    PROC_FILE_TABLE_OFFSET,026
        .equ    PROC_CRED_OFFSET,066
        .equ    PROC_UMASK_OFFSET,067
        .equ    PROC_USTACK_BASE,070
        .equ    PROC_UAREA_OWNER_BASE,01000
        .equ    MM_TYPE_KERNEL_DYNAMIC,3
        .equ    EXEC_DXR_STACK_WORDS,02000
        .equ    RUN_VERSION,2
        .equ    RUN_MIN_WORDS,010
        .equ    RUN_FIXED_WORDS,6
        .equ    RUN_MAX_FDMAP,020
        .equ    RUN_MAX_ARGC,020
        .equ    RUN_MAX_ENVC,020
        .equ    RUN_MAX_PATH_CHARS,0146
        .equ    RUN_MAX_ARG_CHARS,0146
        .equ    PROC_TTY_COUNT,025
        .equ    PROC_ROOT_RESERVED_SLOTS,2

        .text
        .globl  proc_run_block
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot
        .globl  proc_current_slot
        .globl  file_table
        .globl  exec_load_process
        .globl  mm_alloc_aligned
        .globl  fs_zero_words
        .globl  pipe_add_refs
        .globl  proc_slot_discard
        .globl  proc_child_hierarchy
        .globl  proc_scope_id
        .globl  proc_tty_records
        .globl  proc_runq_add
        .globl  vm_space_startup
        .globl  sixbit_record_words

proc_run_block:
        add     17,kconst_6_6
        movei   0,-5(17)
        hrli    0,010
        blt     0,(17)
        move    010,1
        move    011,2

        jumpe   010,proc_run_bad
        skipn   proc_table
        jrst    proc_run_bad
        skipn   proc_current_slot
        jrst    proc_run_bad
        caige   011,RUN_MIN_WORDS
        jrst    proc_run_bad
        hlrz    3,(010)
        caie    3,RUN_VERSION
        jrst    proc_run_bad
        hrrz    3,(010)
        caige   3,RUN_MIN_WORDS
        jrst    proc_run_bad
        camle   3,011
        jrst    proc_run_bad
        move    011,3

        ; Every scalar count is a nonnegative 18-bit value.
        move    4,3(010)
        hrrz    3,4
        came    3,4
        jrst    proc_run_bad
        caile   3,RUN_MAX_FDMAP
        jrst    proc_run_bad
        move    4,4(010)
        hrrz    3,4
        came    3,4
        jrst    proc_run_bad
        caile   3,RUN_MAX_ARGC
        jrst    proc_run_bad
        move    4,5(010)
        hrrz    3,4
        came    3,4
        jrst    proc_run_bad
        caile   3,RUN_MAX_ENVC
        jrst    proc_run_bad

        ; Validate path and all inline startup records within the mapped block.
        move    7,010
        add     7,011
        movei   014,RUN_FIXED_WORDS(010)
        caml    014,7
        jrst    proc_run_bad
        move    1,014
        movei   2,1
        pushj   17,sixbit_record_words
        jumpe   1,proc_run_bad
        move    015,014
        add     015,1
        camle   015,7
        jrst    proc_run_bad

        hrrz    5,4(010)
        hrrz    6,5(010)
        add     5,6
proc_run_record_scan:
        jumpe   5,proc_run_records_done
        caml    015,7
        jrst    proc_run_bad
        move    1,015
        setz    2,
        pushj   17,sixbit_record_words
        jumpe   1,proc_run_bad
        add     015,1
        camle   015,7
        jrst    proc_run_bad
        sojg    5,proc_run_record_scan
proc_run_records_done:
        hrrz    4,3(010)
        add     4,015
        came    4,7
        jrst    proc_run_bad

        ; Keep the highest slots available to UID 0 so an ordinary account
        ; cannot consume every process descriptor and lock out administration.
        ; No quota state is needed: non-root allocation simply stops before
        ; the reserved tail of the already-bounded process table.
        move    011,proc_slots
        move    4,file_table
        hlrz    4,040(4)               ; u-area 066 credential word
        jumpe   4,proc_run_slot_limit_ready
        subi    011,PROC_ROOT_RESERVED_SLOTS
proc_run_slot_limit_ready:
        movei   012,1
        move    013,proc_table
        addi    013,PROC_WORDS
proc_run_slot_loop:
        move    4,2(013)
        tlne    4,PROC_STATE_LH_MASK
        jrst    proc_run_slot_next
        pushj   17,proc_run_id_in_use
        jumpn   1,proc_run_slot_next
        move    4,proc_current_slot
        lsh     4,010
        movem   4,(013)
        setzm   1(013)
        movsi   4,PROC_SCHED_SIDL_LH
        movem   4,2(013)
        movei   4,1(012)
        camle   4,proc_high_slot
        movem   4,proc_high_slot
        jrst    proc_run_slot_found
proc_run_slot_next:
        addi    013,PROC_WORDS
        aos     012
        caml    012,011
        jrst    proc_run_bad
        jrst    proc_run_slot_loop

; Return nonzero if the candidate slot in AC12 is still a live scope ID.
; Derive lifetime from compact process/TTY state; allocate no permanent table.
proc_run_id_in_use:
        movei   4,1                    ; first user slot
        move    3,proc_table
        addi    3,PROC_WORDS
proc_run_id_scan:
        caml    4,proc_high_slot
        jrst    proc_run_id_tty_begin
        hrrz    2,(3)
        andi    2,0377                  ; process group
        camn    2,012
        jrst    kret_one
        move    1,3
        pushj   17,proc_scope_id        ; packed domain,,session in RH
        move    2,1
        andi    2,0377
        camn    2,012
        jrst    kret_one
        lsh     1,-010
        andi    1,0377
        camn    1,012
        jrst    kret_one
        addi    3,PROC_WORDS
        aoja    4,proc_run_id_scan
proc_run_id_tty_begin:
        movei   4,PROC_TTY_COUNT-1
proc_run_id_tty_loop:
        move    2,proc_tty_records(4)
        andi    2,0377                  ; controlling session
        camn    2,012
        jrst    kret_one
        sojge   4,proc_run_id_tty_loop
        jrst    kret_zero
proc_run_slot_found:
        move    1,013
        move    2,012
        move    3,014
        pushj   17,exec_load_process
        jumpge  1,proc_run_exec_ok
        came    1,[-2]
        jrst    proc_run_exec_bad
        movei   2,011
        jrst    proc_run_claimed_bad
proc_run_exec_ok:
        hlrz    011,(013)

        add     17,kconst_3_3
        setzm   -2(17)
        movei   4,-2(17)
        movem   4,-1(17)
        setzm   (17)
        movei   1,PROC_UAREA_WORDS
        movei   2,1
        movei   3,MM_TYPE_KERNEL_DYNAMIC
        movei   4,PROC_UAREA_OWNER_BASE(012)
        pushj   17,mm_alloc_aligned
        move    014,-2(17)
        sub     17,kconst_3_3
        jumpn   1,proc_run_mm_bad
        jumpe   014,proc_run_mm_bad

        move    1,014
        movei   2,PROC_UAREA_WORDS
        pushj   17,fs_zero_words
.if PROC_STACK_WATERMARK
        move    1,014
        addi    1,PROC_USTACK_BASE+1
        move    2,014
        addi    2,PROC_UAREA_WORDS
proc_run_watermark_loop:
        movem   1,(1)
        addi    1,1
        camge   1,2
        jrst    proc_run_watermark_loop
.endif

        ; Locate first argv record and let the machine VM backend relocate
        ; argv/environment into the child image.
        movei   6,RUN_FIXED_WORDS(010)
        move    1,6
        movei   2,1
        pushj   17,sixbit_record_words
        jumpe   1,proc_run_start_record_bad
        add     6,1
        add     17,kconst_4_4
        move    1,013
        move    2,6
        hrrz    3,4(010)
        lsh     3,022
        hrrz    4,5(010)
        ior     3,4
        movei   4,-3(17)
        pushj   17,vm_space_startup
        jumpn   1,proc_run_startup_failed
        move    4,-3(17)
        movem   4,1(014)
        move    4,-2(17)
        movem   4,2(014)
        move    4,-1(17)
        movem   4,3(014)
        move    4,(17)
        movem   4,017(014)
        sub     17,kconst_4_4
        hrrz    4,011
        tlo     4,010000
        movem   4,020(014)
        movei   4,PROC_USTACK_BASE(014)
        movem   4,021(014)

        hrlm    014,(013)
        movei   4,PROC_F_UAREA_RH
        iorm    4,(013)

        ; Inherit SESSION/DOMAIN and resolve INHERIT/NEW/JOIN process-group
        ; semantics only after the child owns a stable u-area.  The helper
        ; also verifies that JOIN names an existing group in this session.
        move    1,012
        move    2,1(010)
        move    3,2(010)
        pushj   17,proc_child_hierarchy
        jumpn   1,proc_run_hierarchy_bad

        skipn   7,file_table
        jrst    proc_run_filetable_bad
        move    4,-1(7)
        movem   4,PROC_FILE_CWD_OFFSET(014)
        ; file_table points at parent u-area 026; credentials/umask are 066/067.
        move    4,040(7)
        movem   4,PROC_CRED_OFFSET(014)
        move    4,041(7)
        movem   4,PROC_UMASK_OFFSET(014)
        setz    4,                      ; child-fd duplicate bitmap
        hrrz    3,3(010)
        jumpe   3,proc_run_map_done
proc_run_map_loop:
        move    2,(015)
        move    1,2
        and     1,[017,,017]
        came    1,2
        jrst    proc_run_fdmap_bad
        hlrz    1,2
        andi    1,017                   ; child fd 0..15
        hrrz    2,2
        andi    2,017                   ; parent fd 0..15
        movei   7,1
        lsh     7,0(1)
        tdne    4,7
        jrst    proc_run_fdmap_bad
        ior     4,7

        move    7,2
        lsh     7,1
        add     7,file_table
        skipn   (7)
        jrst    proc_run_fdmap_bad
        lsh     1,1
        add     1,014
        addi    1,PROC_FILE_TABLE_OFFSET
        move    2,(7)
        movem   2,(1)
        move    2,1(7)
        movem   2,1(1)
proc_run_map_next:
        addi    015,1
        sojg    3,proc_run_map_loop
proc_run_map_done:
        movei   1,PROC_FILE_TABLE_OFFSET(014)
        pushj   17,pipe_add_refs
        movsi   4,PROC_SCHED_SRUN_LH
        movem   4,2(013)
        move    1,012
        pushj   17,proc_runq_add
        move    1,012
        jrst    proc_run_return

proc_run_startup_failed:
        sub     17,kconst_4_4
        jrst    proc_run_claimed_bad

proc_run_exec_bad:
proc_run_mm_bad:
proc_run_start_record_bad:
proc_run_startup_bad:
proc_run_hierarchy_bad:
proc_run_filetable_bad:
proc_run_fdmap_bad:
        jrst    proc_run_claimed_bad

proc_run_claimed_bad:
        move    1,012
        pushj   17,proc_slot_discard
proc_run_bad:
        seto    1,
proc_run_return:
        movei   0,010
        hrli    0,-5(17)
        blt     0,015
        sub     17,kconst_6_6
        popj    17,
