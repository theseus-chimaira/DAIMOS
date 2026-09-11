; syscall_dispatch.s -- native PDP-6 monitor-UUO syscall dispatcher.
;
; Monitor UUOs 040..077 are the conventional userspace syscall ABI.  UUO 043
; is the bulk character-stream write path.  The hardware
; leaves the trapped UUO at 000040 and its computed effective address at
; 000041.  mach_user materializes that effective address in AC1, so real
; arguments arrive here in AC1..AC4 and AC1 also carries the result.

        .text
        .globl  pdp10_ret_zero
        .globl  mach_user_apr
sys_user_words:
        ; User pointers are logical.  Validate against the cached PDP-6 APR
        ; relocation/protection state and translate once for executive access.
        ; The APR LH contains (user_words - 02000), RH the physical base.
        hrrz    1,1
        caige   1,020
        jrst    pdp10_ret_zero
        hlrz    3,mach_user_apr
        addi    3,02000
        caml    1,3
        jrst    pdp10_ret_zero
        hrrz    4,mach_user_apr
        add     1,4
        popj    17,

        .globl  exec_native_syscall
        .globl  proc_nice_current
        .globl  proc_exit_current
        .globl  proc_run_block
        .globl  proc_wait_status
        .globl  proc_control
        .globl  proc_tty_read_enter
        .globl  proc_tty_input
        .globl  proc_current_slot
        .globl  file_stdio_enabled
exec_native_syscall:
        ; Recover the monitor-UUO opcode from the trapped instruction.
        ; AC0 cannot be an index register on the PDP-6: index field zero
        ; means no indexing.  Use caller-scratch AC5 for the table selector.
        hlrz    5,000040
        lsh     5,-011
        subi    5,040
        jumpl   5,%L137
        caile   5,037                   ; opcodes 040..077 inclusive
        jrst    %L137
        jrst    @exec_native_table(5)
exec_native_table:
        .word   %L66                    ; 040 EXIT
        .word   %L67                    ; 041 OPEN
        .word   %L72                    ; 042 CLOSE
        .word   native_sys_write_chars  ; 043 WRITE_CHARS
        .word   native_sys_getchar      ; 044 GETCHAR
        .word   %L75                    ; 045 CHDIR
        .word   %L80                    ; 046 GETCWD
        .word   %L90                    ; 047 STAT
        .word   %L97                    ; 050 DIRREAD
        .word   %L102                   ; 051 MKDIR
        .word   %L107                   ; 052 UNLINK
        .word   %L112                   ; 053 RENAME
        .word   %L119                   ; 054 TRUNCATE
        .word   %L83                    ; 055 READ_WORDS
        .word   %L86                    ; 056 WRITE_WORDS
        .word   %L124                   ; 057 PROCINFO
        .word   %L129                   ; 060 MEMINFO
        .word   %L134                   ; 061 READCHAR
        .word   %L135                   ; 062 WRITECHAR
        .word   %L136                   ; 063 HALT
        .word   native_sys_chmod        ; 064 CHMOD
        .word   native_sys_dtfs_format  ; 065 DTFS_FORMAT/CHECK
        .word   native_sys_dtfs_mount   ; 066 DTFS_MOUNT
        .word   native_sys_unmount      ; 067 UNMOUNT
        .word   native_sys_flock        ; 070 FLOCK
        .word   native_sys_dup          ; 071 DUP
        .word   native_sys_symlink      ; 072 SYMLINK
        .word   native_sys_nice         ; 073 NICE
        .word   native_sys_run          ; 074 RUN
        .word   native_sys_wait         ; 075 WAIT
        .word   native_sys_getpid       ; 076 GETPID
        .word   native_sys_procctl       ; 077 PROCCTL

; UUO 043 WRITE_CHARS: AC1 console fd, AC2 9-bit byte pointer, AC3 chars.
; This is deliberately the console fast path only.  Regular-file stream writes
; retain WRITECHAR semantics in libc; CAT uses this call only for console output.
native_sys_write_chars:
        push    17,2                  ; preserve user byte pointer
        push    17,3                  ; preserve character count
        hrrz    1,1
        caige   1,1
        jrst    native_sys_write_chars_stdio_bad
        caile   1,2
        jrst    native_sys_write_chars_stdio_bad
        pushj   17,file_stdio_enabled
        jumpe   1,native_sys_write_chars_stdio_bad
        pop     17,3
        pop     17,2
        hrrz    6,3                   ; character count
        jumpe   6,native_sys_write_chars_ok
        move    5,2                   ; logical 9-bit byte pointer
        hrrz    1,2
        pushj   17,sys_user_words
        jumpe   1,native_sys_write_chars_fail
        hrr     5,1                   ; translated byte pointer
        add     3,4                   ; one-past physical user end
        move    7,3

native_sys_write_chars_loop:
        hrrz    4,5
        caml    4,7
        jrst    native_sys_write_chars_fail
        ldb     1,5
        pushj   17,native_sys_putchar
        jumpn   1,native_sys_write_chars_fail
        soje    6,native_sys_write_chars_ok
        ibp     5
        jrst    native_sys_write_chars_loop
native_sys_write_chars_ok:
        setz    1,
        popj    17,
native_sys_write_chars_stdio_bad:
        pop     17,3
        pop     17,2
native_sys_write_chars_fail:
        seto    1,
        popj    17,

%L66:
        push    17,1
        pushj   17,file_close_all
        pop     17,1
        ; EXIT never returns through the dying process's u-area stack.
        jrst    proc_exit_current
%L67:
        ; AC1 path, AC2 flags.
        pushj   17,sys_user_words
        jumpe   1,%L137
        hrrz    3,2
        move    4,3
        andi    4,3
        addi    4,1
        caile   4,3
        movei   4,3
        andi    3,034
        ior     3,4
        move    2,3
        jrst    file_open
%L72:
        hrrz    1,1
        jrst    file_close
%L75:
        pushj   17,sys_user_words
        jumpe   1,%L137
        jrst    file_chdir
%L80:
        pushj   17,sys_user_words
        jumpe   1,%L137
        hrrz    2,2
        jrst    file_getcwd
%L83:
        ; AC1 fd, AC2 buffer, AC3 word count.
        move    6,1
        move    7,3
        move    1,2
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,1
        hrrz    1,6
        hrrz    3,7
        jrst    file_read_words
%L86:
        ; AC1 fd, AC2 buffer, AC3 word count, AC4 buffer size.
        move    6,1
        move    7,3
        move    5,4
        move    1,2
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,1
        hrrz    1,6
        hrrz    3,7
        move    4,5
        jrst    file_write_words
%L90:
        ; AC1 path, AC2 stat buffer.
        pushj   17,sys_user_words
        move    5,1
        move    1,2
        pushj   17,sys_user_words
        jumpe   5,%L137
        jumpe   1,%L137
        move    2,1
        move    1,5
        jrst    file_stat_path
%L97:
        ; AC1 fd, AC2 directory entry buffer.
        move    5,1
        move    1,2
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,1
        hrrz    1,5
        jrst    file_readdir
%L102:
        ; AC1 path, AC2 mode.
        pushj   17,sys_user_words
        jumpe   1,%L137
        hrrz    2,2
        jrst    file_mkdir
%L107:
        pushj   17,sys_user_words
        jumpe   1,%L137
        jrst    file_unlink
%L112:
        ; AC1 old path, AC2 new path.
        pushj   17,sys_user_words
        move    5,1
        move    1,2
        pushj   17,sys_user_words
        jumpe   5,%L137
        jumpe   1,%L137
        move    2,1
        move    1,5
        jrst    file_rename
%L119:
        ; AC1 path, AC2 new size.
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,2
        jrst    file_truncate
%L124:
        ; AC1 slot, AC2 result buffer.
        move    5,1
        move    1,2
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,1
        hrrz    1,5
        jrst    sys_procinfo
%L129:
        pushj   17,sys_user_words
        jumpe   1,%L137
        jrst    sys_meminfo
%L134:
        hrrz    1,1
        jumpe   1,native_sys_readchar_stdio
        pushj   17,file_readchar
        camn    1,[-3]
        jrst    native_sys_getchar
        jrst    %L65
%L135:
        ; AC1 fd, AC2 character.  Preserve the character on the process
        ; kernel stack across a real file_writechar call.
        hrrz    1,1
        cail    1,1
        cail    1,3
        trna
        jrst    native_sys_writechar_tty
        push    17,2
        andi    2,0777
        pushj   17,file_writechar
        pop     17,2
        came    1,[-3]
        jrst    %L65
native_sys_writechar_tty:
        push    17,2
        pushj   17,file_stdio_enabled
        pop     17,2
        jumpe   1,%L137
        move    1,2
        andi    1,0777
        jrst    native_sys_putchar

native_sys_readchar_stdio:
        movei   1,0
        pushj   17,file_stdio_enabled
        jumpe   1,%L137
        jrst    native_sys_getchar

; Return the DTC0 vnode for a valid translated user path, or zero on failure.
; A one-word process-private kernel-stack temporary replaces the old global
; syscall AC5 shadow and remains safe across scheduler sleep/resume.
native_sys_dtc0_path:
        pushj   17,sys_user_words
        jumpe   1,pdp10_ret_zero
        push    17,0
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,native_sys_dtc0_bad
        move    5,(17)
        pop     17,0
        move    1,5
        came    1,[020003000014]        ; DEVICEFS DTC0 directory
        jrst    pdp10_ret_zero
        popj    17,
native_sys_dtc0_bad:
        pop     17,0
        jrst    pdp10_ret_zero

native_sys_chmod:
        ; Preserve mode across pointer translation and VFS lookup.
        push    17,2
        pushj   17,sys_user_words
        jumpe   1,native_sys_chmod_bad1
        push    17,0
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,native_sys_chmod_bad2
        move    5,(17)
        pop     17,0
        pop     17,2
        move    1,5
        hrrz    2,2
        jrst    vfs_chmod
native_sys_chmod_bad2:
        pop     17,0
native_sys_chmod_bad1:
        pop     17,0
        jrst    %L137

native_sys_dtfs_format:
        ; AC1 device path, AC2 DTFS management control word.
        push    17,2
        pushj   17,native_sys_dtc0_path
        pop     17,2
        jumpe   1,%L137
        hrrz    2,2
        move    3,2
        andi    3,07
        caile   3,1
        jrst    %L137
        movei   1,0                     ; DTC0 unit
        .globl  sys_dtfs_format_jump
sys_dtfs_format_jump:
        jrst    pdp10_ret_neg1

native_sys_dtfs_mount:
        ; AC1 device path, AC2 mount path, AC3 flags.
        push    17,2                    ; preserve mount path
        push    17,3                    ; preserve flags
        pushj   17,native_sys_dtc0_path
        jumpe   1,native_sys_dtfs_mount_bad2
        move    1,-1(17)                ; saved mount path
        pushj   17,sys_user_words
        jumpe   1,native_sys_dtfs_mount_bad2
        pop     17,3                    ; restore flags
        pop     17,0                    ; discard saved mount path
        push    17,3                    ; preserve flags across VFS lookup
        push    17,0                    ; vnode/result scratch
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,native_sys_dtfs_mount_bad_lookup
        move    2,(17)                  ; mounted-on root vnode
        move    3,-1(17)                ; saved flags
        hrrz    3,3
        caile   3,031                   ; RO plus DTFS type override
        jrst    native_sys_dtfs_mount_bad_lookup
        movei   1,0                     ; DTC0 unit
        movei   4,(17)                  ; returned root scratch
        .globl  sys_dtfs_mount_jump
sys_dtfs_mount_jump:
        pushj   17,pdp10_ret_neg1
        pop     17,0                    ; result scratch
        pop     17,0                    ; saved flags
        popj    17,
native_sys_dtfs_mount_bad_lookup:
        pop     17,0
        pop     17,0
        jrst    %L137
native_sys_dtfs_mount_bad2:
        pop     17,0
        pop     17,0
        jrst    %L137

native_sys_unmount:
        pushj   17,sys_user_words
        jumpe   1,%L137
        push    17,0
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,native_sys_unmount_bad
        move    5,(17)
        pop     17,0
        move    1,5
        jrst    vfs_unmount
native_sys_unmount_bad:
        pop     17,0
        jrst    %L137

native_sys_flock:
        hrrz    1,1
        hrrz    2,2
        jrst    file_lock

native_sys_dup:
        hrrz    1,1
        jrst    file_dup

native_sys_symlink:
        ; AC1 target, AC2 link path.
        pushj   17,sys_user_words
        move    5,1
        move    1,2
        pushj   17,sys_user_words
        jumpe   5,%L137
        jumpe   1,%L137
        move    2,1
        move    1,5
        jrst    file_symlink

native_sys_nice:
        ; UUO effective addresses are 18-bit.  NICE is the one current arg0
        ; with signed semantics, so restore the 36-bit C value explicitly.
        hrrz    1,1
        trnn    1,0400000
        jrst    proc_nice_current
        tlo     1,0777777
        jrst    proc_nice_current

native_sys_run:
        ; AC1 points at an inline, versioned RUN block.  Translate once and
        ; pass the number of user words remaining after that address as AC2.
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,4
        add     2,3
        sub     2,1
        jrst    proc_run_block

native_sys_wait:
        ; AC1 selector, AC2 optional status word pointer, AC3 flags.
        move    5,1
        move    6,3
        move    1,2
        jumpe   1,native_sys_wait_no_status
        pushj   17,sys_user_words
        jumpe   1,%L137
        move    2,1
        jrst    native_sys_wait_call
native_sys_wait_no_status:
        setz    2,
native_sys_wait_call:
        hrrz    1,5
        hrrz    3,6
        jrst    proc_wait_status

native_sys_getpid:
        move    1,proc_current_slot
        popj    17,

native_sys_procctl:
        hrrz    1,1
        hrrz    2,2
        jrst    proc_control

native_sys_getchar:
        movei   1,0
        pushj   17,file_stdio_enabled
        jumpe   1,%L137
native_sys_getchar_policy:
        movei   1,0                    ; logical CTY id
        pushj   17,proc_tty_read_enter
        jumpn   1,%L137
native_sys_getchar_again:
        pushj   17,native_sys_getchar_call
        jumpl   1,%L65                 ; no installed CTY input service
        move    2,1                    ; raw character
        movei   1,0                    ; logical CTY id
        pushj   17,proc_tty_input
        camn    1,[-2]                 ; consumed job-control character
        jrst    native_sys_getchar_policy
        jrst    %L65

        ; MINIT patches this one-word call target to CTY getchar.  Keep it as
        ; a callable trampoline so terminal policy remains in KCORE around the
        ; relocatable CTY MRES implementation.
        .globl  native_sys_getchar_call
native_sys_getchar_call:
        pushj   17,pdp10_ret_neg1
        popj    17,

native_sys_putchar:
        .globl  native_sys_putchar_call
native_sys_putchar_call:
        pushj   17,pdp10_ret_neg1
        jrst    %L65
%L136:
        halt    .
        seto    1,
        jrst    %L65
%L137:
        seto    1,
; Leave the native syscall result in AC1 for mach_syscall.
%L65:
        popj    17,
