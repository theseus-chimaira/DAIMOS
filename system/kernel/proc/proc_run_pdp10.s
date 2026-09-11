; proc_run_pdp10.s -- compact resident RUN construction for PDP-6.
;
; RUN is implemented here rather than keeping the boot-time C constructor
; resident.  The three-word process descriptor remains the permanent table
; cost.  The child u-area contains explicit descriptor inheritance state.
;
; C ABI entry:
;   AC1 = physical address of struct sys_run_v1
;   AC2 = user words available from AC1 through end of user extent
; Return AC1 = child PID/slot, or -1.

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000
        .equ    PROC_SCHED_SIDL_LH,0100024
        .equ    PROC_SCHED_SRUN_LH,0200024
        .equ    PROC_F_UAREA_RH,0400000
        .equ    PROC_UAREA_WORDS,0420
        .equ    PROC_FDCTL_OFFSET,045
        .equ    PROC_FILE_CWD_OFFSET,046
        .equ    PROC_FILE_TABLE_OFFSET,047
        .equ    PROC_USTACK_BASE,0101
        .equ    PROC_UAREA_OWNER_BASE,01000
        .equ    MM_TYPE_KERNEL_DYNAMIC,3
        .equ    EXEC_DXR_STACK_WORDS,02000
        .equ    RUN_MIN_WORDS,011
        .equ    RUN_FIXED_WORDS,7
        .equ    RUN_MAX_FDMAP,020
        .equ    RUN_MAX_PATH_CHARS,0146

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
        .globl  proc_slot_discard
        .globl  proc_child_hierarchy

proc_run_block:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
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
        caie    3,1
        jrst    proc_run_bad
        hrrz    3,(010)
        caige   3,RUN_MIN_WORDS
        jrst    proc_run_bad
        camle   3,011
        jrst    proc_run_bad
        move    011,3
        move    4,3(010)
        hrrz    3,4
        came    3,4
        jrst    proc_run_bad
        caile   3,RUN_MAX_FDMAP
        jrst    proc_run_bad

        move    014,010
        addi    014,RUN_FIXED_WORDS
        move    4,(014)
        hrrz    5,4
        came    4,5
        jrst    proc_run_bad
        jumpe   4,proc_run_bad
        caile   4,RUN_MAX_PATH_CHARS
        jrst    proc_run_bad
        addi    4,5
        move    5,4
        setz    4,
        divi    4,6
        addi    4,1
        move    015,014
        add     015,4
        addi    4,RUN_FIXED_WORDS
        add     4,3(010)
        came    4,011
        jrst    proc_run_bad

        movei   012,1
        move    013,proc_table
        addi    013,PROC_WORDS
proc_run_slot_loop:
        move    4,2(013)
        tlne    4,PROC_STATE_LH_MASK
        jrst    proc_run_slot_next
        move    4,proc_current_slot
        lsh     4,010
        movem   4,(013)
        setzm   1(013)
        movsi   4,PROC_SCHED_SIDL_LH
        movem   4,2(013)
        move    4,012
        addi    4,1
        camle   4,proc_high_slot
        movem   4,proc_high_slot
        jrst    proc_run_slot_found
proc_run_slot_next:
        addi    013,PROC_WORDS
        aos     012
        caml    012,proc_slots
        jrst    proc_run_bad
        jrst    proc_run_slot_loop

proc_run_slot_found:
        move    1,013
        move    2,012
        move    3,014
        pushj   17,exec_load_process
        jumpn   1,proc_run_claimed_bad
        hlrz    011,(013)

        add     17,[3,,3]
        setzm   -2(17)
        movei   4,-2(17)
        movem   4,-1(17)
        setzm   (17)
        movei   1,PROC_UAREA_WORDS
        movei   2,1
        movei   3,MM_TYPE_KERNEL_DYNAMIC
        move    4,012
        addi    4,PROC_UAREA_OWNER_BASE
        pushj   17,mm_alloc_aligned
        move    014,-2(17)
        sub     17,[3,,3]
        jumpn   1,proc_run_claimed_bad
        jumpe   014,proc_run_claimed_bad

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

        move    4,4(010)
        movem   4,1(014)
        move    4,5(010)
        movem   4,2(014)
        move    4,6(010)
        movem   4,3(014)
        hlrz    4,1(013)
        subi    4,EXEC_DXR_STACK_WORDS+1
        movem   4,017(014)
        hrrz    4,011
        tlo     4,010000
        movem   4,020(014)
        move    4,014
        addi    4,PROC_USTACK_BASE
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
        jumpn   1,proc_run_claimed_bad

        skipn   7,file_table
        jrst    proc_run_claimed_bad
        move    4,-1(7)
        movem   4,PROC_FILE_CWD_OFFSET(014)
        move    6,-2(7)
        andi    6,7
        setzb   4,5
        hrrz    3,3(010)
        jumpe   3,proc_run_map_done
proc_run_map_loop:
        move    2,(015)
        move    1,2
        and     1,[017,,017]
        came    1,2
        jrst    proc_run_claimed_bad
        hlrz    1,2
        andi    1,017
        hrrz    2,2
        andi    2,017
        movei   7,1
        lsh     7,0(1)
        tdne    4,7
        jrst    proc_run_claimed_bad
        ior     4,7
        caige   1,3
        jrst    proc_run_map_stdio
        caige   2,3
        jrst    proc_run_claimed_bad

        move    7,2
        subi    7,3
        lsh     7,1
        add     7,file_table
        skipn   (7)
        jrst    proc_run_claimed_bad
        subi    1,3
        lsh     1,1
        add     1,014
        addi    1,PROC_FILE_TABLE_OFFSET
        move    2,(7)
        movem   2,(1)
        move    2,1(7)
        movem   2,1(1)
        jrst    proc_run_map_next
proc_run_map_stdio:
        caige   2,3
        jrst    proc_run_map_stdio_low
        jrst    proc_run_claimed_bad
proc_run_map_stdio_low:
        came    1,2
        jrst    proc_run_claimed_bad
        tdnn    6,7
        jrst    proc_run_claimed_bad
        ior     5,7
proc_run_map_next:
        addi    015,1
        sojg    3,proc_run_map_loop
proc_run_map_done:
        ; Preserve SESSION/DOMAIN already installed by proc_child_hierarchy.
        iorm    5,PROC_FDCTL_OFFSET(014)
        movsi   4,PROC_SCHED_SRUN_LH
        movem   4,2(013)
        move    1,012
        jrst    proc_run_return

proc_run_claimed_bad:
        move    1,012
        pushj   17,proc_slot_discard
proc_run_bad:
        seto    1,
proc_run_return:
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
