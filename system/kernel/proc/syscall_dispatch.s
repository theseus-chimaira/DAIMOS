; syscall_dispatch.s -- native PDP-6 monitor-UUO syscall dispatcher.
;
; Monitor UUOs 040..077 are the conventional userspace syscall ABI.  UUO 043
; is the bulk character-stream write path.  The hardware
; leaves the trapped UUO at 000040 and its computed effective address at
; 000041.  mach_user materializes that effective address in AC1, so real
; arguments arrive here in AC1..AC4 and AC1 also carries the result.

        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  vm_user_words
        .globl  vm_user_mapping_hold
        .globl  vm_user_mapping_release

        .globl  exec_native_syscall
        .globl  proc_nice_current
        .globl  proc_exit_current
        .globl  proc_run_block
        .globl  proc_wait_status
        .globl  proc_control
        .globl  proc_tty_read_enter
        .globl  proc_tty_input
        .globl  proc_tty_line_take
        .globl  proc_tty_output
        .globl  proc_current_slot
        .globl  pipe_create
        .globl  file_mkfifo
        .globl  file_writechar_reserve
        .globl  exec_replace_current
        .globl  proc_exec_enter
        .globl  pclk_time36
        .globl  vfs_utime
        .globl  vfs_chown
        .globl  file_rmdir
        .globl  file_check_root
exec_native_syscall:
        ; Recover the monitor-UUO opcode from the trapped instruction.
        ; AC0 cannot be an index register on the PDP-6: index field zero
        ; means no indexing.  Use caller-scratch AC5 for the table selector.
        hlrz    5,000040
        lsh     5,-011
        subi    5,040
        ; Hardware monitor UUOs reaching mach_syscall are exactly 040..077.
        ; Two 18-bit handler addresses share each permanent dispatch word.
        move    6,5
        andi    5,1
        lsh     6,-1
        xct     exec_native_half_select(5)
        jrst    (5)
exec_native_half_select:
        hlrz    5,exec_native_table(6)
        hrrz    5,exec_native_table(6)
exec_native_table:
        .word   %L66,,%L67
        .word   %L72,,native_sys_write_chars
        .word   native_sys_getchar,,%L75
        .word   %L80,,%L90
        .word   %L97,,%L102
        .word   %L107,,%L112
        .word   %L119,,%L83
        .word   %L86,,%L124
        .word   %L129,,%L134
        .word   %L135,,%L136
        .word   native_sys_chmod,,native_sys_dtfs_format
        .word   native_sys_dtfs_mount,,native_sys_unmount
        .word   native_sys_flock,,native_sys_dup
        .word   native_sys_symlink,,native_sys_nice
        .word   native_sys_run,,native_sys_wait
        .word   native_sys_getpid,,native_sys_extctl

native_sys_getpid:
        move    1,proc_current_slot
        popj    17,

; Translate one user pointer while marking the current user extent immovable.
; Success returns the mapped pointer in AC1 with the hold still active.  A
; failed translation drops the hold before returning zero.
native_sys_map_one:
        pushj   17,vm_user_mapping_hold
        pushj   17,vm_user_words
        jumpn   1,native_sys_map_one_ok
        jrst    vm_user_mapping_release
native_sys_map_one_ok:
        popj    17,

; Common return for syscalls that retained a physical user mapping across a
; potentially blocking kernel call.  The release helper preserves AC1.
native_sys_mapped_return:
        jrst    vm_user_mapping_release

; UUO 043 WRITE_CHARS: AC1 fd, AC2 9-bit byte pointer, AC3 chars.
; Preserve the one-trap bulk ABI for ordinary files and future pipe streams.
; CTY remains fast enough for bring-up; the later pipe bulk step may specialize
; device/pipe transfer after measurements without changing this ABI.
native_sys_write_chars:
        ; AC10-12 carry the translated stream cursor across pipe calls.
        ; They are callee-saved by the PDP-10 C ABI; the original user values
        ; are restored before leaving the syscall.
        push    17,010
        push    17,011
        push    17,012
        push    17,1                  ; fd
        push    17,2                  ; user byte pointer
        push    17,3                  ; chunk counter scratch
        hrrz    011,3                 ; characters remaining
        jumpe   011,native_sys_write_chars_empty
        move    010,2
        hrrz    1,2
        pushj   17,native_sys_map_one
        jumpe   1,native_sys_write_chars_map_fail
        hrr     010,1                 ; translated byte pointer
        add     3,4                   ; one-past physical user end
        move    012,3

        ; The first pipe character in each <= PIPE_BUF chunk waits for enough
        ; room for the complete chunk.  The remaining calls reduce that
        ; reservation one character at a time.  Executive code does not
        ; schedule another process between these non-waiting calls, preserving
        ; atomicity without a separate pipe reservation object.
native_sys_write_chars_chunk:
        move    1,011
        caile   1,0200                ; PIPE_BUF = 128 characters
        movei   1,0200
        movem   1,(17)

native_sys_write_chars_loop:
        hrrz    4,010
        caml    4,012
        jrst    native_sys_write_chars_fail
        ldb     2,010
        move    1,-2(17)              ; saved fd
        hrrz    1,1
        move    3,(17)                ; remaining atomic reservation
        pushj   17,file_writechar_reserve
        camn    1,[-3]                ; VFS_DEVICE_IO
        jrst    native_sys_write_chars_device
        jumpn   1,native_sys_write_chars_fail
native_sys_write_chars_next:
        soje    011,native_sys_write_chars_ok
        ibp     010
        sosle   (17)
        jrst    native_sys_write_chars_loop
        jrst    native_sys_write_chars_chunk
native_sys_write_chars_device:
        move    1,2
        pushj   17,native_sys_putchar
        jumpn   1,native_sys_write_chars_fail
        jrst    native_sys_write_chars_next
native_sys_write_chars_empty:
        setz    1,
        jrst    native_sys_write_chars_restore
native_sys_write_chars_ok:
        setz    1,
native_sys_write_chars_done:
        pushj   17,vm_user_mapping_release
native_sys_write_chars_restore:
        sub     17,[3,,3]
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
native_sys_write_chars_fail:
        seto    1,
        jrst    native_sys_write_chars_done
native_sys_write_chars_map_fail:
        seto    1,
        jrst    native_sys_write_chars_restore

%L66:
        push    17,1
        pushj   17,file_close_all
        pop     17,1
        ; EXIT never returns through the dying process's u-area stack.
        jrst    proc_exit_current
%L67:
        ; AC1 path, AC2 flags.
        pushj   17,native_sys_map_one
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
        pushj   17,file_open
        jrst    native_sys_mapped_return
%L72:
        hrrz    1,1
        jrst    file_close
%L75:
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        pushj   17,file_chdir
        jrst    native_sys_mapped_return
%L80:
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        hrrz    2,2
        pushj   17,file_getcwd
        jrst    native_sys_mapped_return
%L83:
        ; AC1 fd, AC2 buffer, AC3 word count.
        move    6,1
        move    7,3
        move    1,2
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        move    2,1
        hrrz    1,6
        hrrz    3,7
        pushj   17,file_read_words
        jrst    native_sys_mapped_return
%L86:
        ; AC1 fd, AC2 buffer, AC3 word count, AC4 buffer size.
        move    6,1
        move    7,3
        move    5,4
        move    1,2
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        move    2,1
        hrrz    1,6
        hrrz    3,7
        move    4,5
        pushj   17,file_write_words
        jrst    native_sys_mapped_return
; Translate two user pointers in AC1/AC2.  Return mapped pointers in AC1/AC2
; or -1 in AC1.  The three pathname syscalls share this cold validation path.
native_sys_two_paths:
        pushj   17,vm_user_mapping_hold
        pushj   17,vm_user_words
        move    5,1
        move    1,2
        pushj   17,vm_user_words
        jumpe   5,native_sys_two_paths_bad
        jumpe   1,native_sys_two_paths_bad
        move    2,1
        move    1,5
        popj    17,
native_sys_two_paths_bad:
        pushj   17,vm_user_mapping_release
        jrst    pdp10_ret_neg1

%L90:
        ; AC1 path, AC2 stat buffer.
        pushj   17,native_sys_two_paths
        jumpl   1,%L137
        pushj   17,file_stat_path
        jrst    native_sys_mapped_return
%L97:
        ; AC1 fd, AC2 directory entry buffer.
        move    5,1
        move    1,2
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        move    2,1
        hrrz    1,5
        pushj   17,file_readdir
        jrst    native_sys_mapped_return
%L102:
        ; AC1 path, AC2 mode.
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        hrrz    2,2
        pushj   17,file_mkdir
        jrst    native_sys_mapped_return
%L107:
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        pushj   17,file_unlink
        jrst    native_sys_mapped_return
%L112:
        ; AC1 old path, AC2 new path.
        pushj   17,native_sys_two_paths
        jumpl   1,%L137
        pushj   17,file_rename
        jrst    native_sys_mapped_return
%L119:
        ; AC1 path, AC2 new size.
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        pushj   17,file_truncate
        jrst    native_sys_mapped_return
%L124:
        ; AC1 slot, AC2 result buffer.
        move    5,1
        move    1,2
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        move    2,1
        hrrz    1,5
        pushj   17,sys_procinfo
        jrst    native_sys_mapped_return
%L129:
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        pushj   17,sys_meminfo
        jrst    native_sys_mapped_return
%L134:
        hrrz    1,1
        pushj   17,file_readchar
        came    1,[-3]
        jrst    %L65
        jrst    native_sys_getchar_policy
%L135:
        ; AC1 fd, AC2 character.  Save only the character for CTY fallback.
        hrrz    1,1
        push    17,2
        andi    2,0777
        pushj   17,file_writechar
        came    1,[-3]
        jrst    native_sys_writechar_done
        move    1,(17)
        andi    1,0777
        pushj   17,native_sys_putchar
native_sys_writechar_done:
        sub     17,[1,,1]
        jrst    %L65

; Translate and resolve one user pathname.  Return its vnode in AC1, or zero.
; The one-word scratch lives on the current process's private kernel stack.
native_sys_lookup_user_path:
        pushj   17,native_sys_map_one
        jumpe   1,pdp10_ret_zero
        push    17,0
        movei   2,(17)
        pushj   17,file_lookup_path
        jumpn   1,native_sys_lookup_user_path_bad
        move    1,(17)
        pop     17,0
        jrst    vm_user_mapping_release
native_sys_lookup_user_path_bad:
        pop     17,0
        pushj   17,vm_user_mapping_release
        jrst    pdp10_ret_zero

; Return the DTC0 vnode for a valid translated user path, or zero on failure.
native_sys_dtc0_path:
        pushj   17,native_sys_lookup_user_path
        came    1,[020002000014]        ; /DEV/DTC0 direct endpoint
        jrst    pdp10_ret_zero
        popj    17,

native_sys_chmod:
        ; Preserve mode across pointer translation and VFS lookup.
        push    17,2
        pushj   17,native_sys_lookup_user_path
        jumpe   1,native_sys_chmod_bad
        push    17,1                   ; vnode
        pushj   17,file_check_owner
        jumpn   1,native_sys_chmod_owner_bad
        pop     17,1
        pop     17,2
        hrrz    2,2
        jrst    vfs_chmod
native_sys_chmod_owner_bad:
        pop     17,0                   ; vnode
native_sys_chmod_bad:
        pop     17,0                   ; saved mode
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
        pushj   17,native_sys_lookup_user_path
        jumpe   1,native_sys_dtfs_mount_bad2
        move    2,1                     ; mounted-on root vnode
        pop     17,3                    ; restore flags
        pop     17,0                    ; discard saved mount path
        hrrz    3,3
        caile   3,031                   ; RO plus DTFS type override
        jrst    %L137
        movei   1,0                     ; DTC0 unit
        push    17,0                    ; returned root scratch
        movei   4,(17)
        .globl  sys_dtfs_mount_jump
sys_dtfs_mount_jump:
        pushj   17,pdp10_ret_neg1
        pop     17,0                    ; result scratch
        popj    17,
native_sys_dtfs_mount_bad2:
        pop     17,0
        pop     17,0
        jrst    %L137

native_sys_unmount:
        pushj   17,native_sys_lookup_user_path
        jumpe   1,%L137
        jrst    vfs_unmount

native_sys_flock:
        hrrz    1,1
        hrrz    2,2
        jrst    file_lock

native_sys_dup:
        hrrz    1,1
        jrst    file_dup

native_sys_symlink:
        ; AC1 target, AC2 link path.
        pushj   17,native_sys_two_paths
        jumpl   1,%L137
        pushj   17,file_symlink
        jrst    native_sys_mapped_return

native_sys_nice:
        ; UUO effective addresses are 18-bit.  NICE is the one current arg0
        ; with signed semantics, so restore the 36-bit C value explicitly.
        hrrz    1,1
        trnn    1,0400000
        jrst    proc_nice_current
        tlo     1,0777777
        jrst    proc_nice_current

native_sys_run:
        ; AC1 points at an inline, versioned RUN block.  Keep the parent extent
        ; fixed while proc_run_block may sleep during executable loading.
        pushj   17,native_sys_map_one
        jumpe   1,%L137
        move    2,4
        add     2,3
        sub     2,1
        pushj   17,proc_run_block
        jrst    native_sys_mapped_return

native_sys_wait:
        ; AC1 selector, AC2 optional logical status pointer, AC3 flags.
        ; WAIT may sleep while the parent itself is swapped or compacted, so
        ; never retain a translated physical status address across the wait.
        move    5,1
        move    6,3
        push    17,2                    ; logical user status pointer
        push    17,0                    ; stable kernel status scratch
        move    1,-1(17)
        jumpe   1,native_sys_wait_call
        pushj   17,vm_user_words        ; validate before sleeping
        jumpe   1,native_sys_wait_bad
native_sys_wait_call:
        hrrz    1,5
        movei   2,(17)
        hrrz    3,6
        pushj   17,proc_wait_status
        move    5,1                     ; preserve returned child PID
        jumpg   5,native_sys_wait_copy
        jrst    native_sys_wait_done
native_sys_wait_copy:
        move    1,-1(17)
        jumpe   1,native_sys_wait_done
        pushj   17,vm_user_words        ; remap after parent resumes
        jumpe   1,native_sys_wait_bad_result
        move    2,(17)
        movem   2,(1)
native_sys_wait_done:
        move    1,5
        sub     17,[2,,2]
        popj    17,
native_sys_wait_bad_result:
        seto    5,
        jrst    native_sys_wait_done
native_sys_wait_bad:
        sub     17,[2,,2]
        jrst    %L137

native_sys_extctl:
        hrrz    1,1
        cain    1,032                  ; SYS_EXT_SEEK
        jrst    native_sys_seek
        cain    1,033                  ; SYS_EXT_CHOWN
        jrst    native_sys_chown
        cain    1,034                  ; SYS_EXT_RMDIR
        jrst    native_sys_rmdir
        cain    1,035                  ; SYS_EXT_UTIME
        jrst    native_sys_utime
        cain    1,020                  ; SYS_EXT_PIPE
        jrst    pipe_create
native_sys_ext_nonpipe:
        cain    1,023                  ; SYS_EXT_GETTIME
        jrst    pclk_time36
        cain    1,024                  ; SYS_EXT_DUP2
        jrst    native_sys_dup2
        cain    1,022                  ; SYS_EXT_EXEC
        jrst    native_sys_exec
        caie    1,021                  ; SYS_EXT_MKFIFO
        jrst    native_sys_procctl
        move    1,2                    ; user path
        push    17,3                   ; preserve mode across VM translation
        pushj   17,native_sys_map_one
        pop     17,2
        jumpe   1,pdp10_ret_neg1
        hrrz    2,2
        pushj   17,file_mkfifo
        jrst    native_sys_mapped_return

; EXEC AC2 points at an inline, versioned launch block.  Five stable
; kernel-stack words receive entry, stack, argc, argv, and envp.  Success has
; committed a new VM and must never return through the old image.
native_sys_exec:
        move    1,2
        pushj   17,native_sys_map_one
        jumpe   1,pdp10_ret_neg1
        move    2,4
        add     2,3
        sub     2,1                    ; mapped parent words available
        add     17,[5,,5]
        movei   3,-4(17)
        pushj   17,exec_replace_current
        jumpn   1,native_sys_exec_bad
        move    1,-4(17)               ; replacement entry
        move    2,-3(17)               ; replacement user stack
        move    3,-2(17)               ; argc
        move    4,-1(17)               ; argv
        move    5,(17)                  ; envp
        sub     17,[5,,5]
        jrst    proc_exec_enter
native_sys_exec_bad:
        sub     17,[5,,5]
        jrst    native_sys_mapped_return

native_sys_dup2:
        hrrz    1,2                    ; old fd
        hrrz    2,3                    ; replacement fd
        jrst    file_dup2

native_sys_seek:
        hrrz    1,2                    ; fd
        move    2,3                    ; signed character offset
        hrrz    3,4                    ; SYS_SEEK_*
        jrst    file_seek

native_sys_chown:
        push    17,2                    ; path
        push    17,3                    ; uid
        push    17,4                    ; gid
        pushj   17,file_check_root
        jumpn   1,native_sys_chown_fail
        move    1,-2(17)               ; path
        pushj   17,native_sys_lookup_user_path
        jumpe   1,native_sys_chown_fail
        move    2,-1(17)               ; uid
        move    3,(17)                 ; gid
        sub     17,[3,,3]
        jrst    vfs_chown
native_sys_chown_fail:
        sub     17,[3,,3]
        jrst    pdp10_ret_neg1

native_sys_rmdir:
        move    1,2
        pushj   17,native_sys_map_one
        jumpe   1,pdp10_ret_neg1
        pushj   17,file_rmdir
        jrst    native_sys_mapped_return

native_sys_utime:
        push    17,3                    ; TIME36
        move    1,2                    ; path
        pushj   17,native_sys_lookup_user_path
        jumpe   1,native_sys_utime_fail
        push    17,1                    ; vnode
        pushj   17,file_check_owner
        jumpn   1,native_sys_utime_owner_fail
        pop     17,1
        pop     17,2                    ; TIME36
        jrst    vfs_utime
native_sys_utime_owner_fail:
        pop     17,0
native_sys_utime_fail:
        sub     17,[1,,1]
        jrst    pdp10_ret_neg1

native_sys_procctl:
        hrrz    2,2
        jrst    proc_control

native_sys_getchar:
        ; Legacy GETCHAR follows descriptor 0 just like READCHAR.
        movei   1,0
        pushj   17,file_readchar
        came    1,[-3]
        popj    17,
native_sys_getchar_policy:
        pushj   17,proc_tty_read_enter
        jumpl   1,%L137
        push    17,1                   ; logical controlling TTY
        pushj   17,proc_tty_line_take  ; drain cooked data before hardware
        came    1,[-3]
        jrst    native_sys_getchar_buffered
        move    1,(17)                 ; restore logical TTY
native_sys_getchar_again:
        pushj   17,native_sys_getchar_call
        jumpl   1,native_sys_getchar_error
        move    2,1                    ; character
        pop     17,1                   ; logical TTY
        pushj   17,proc_tty_input
        camn    1,[-3]                 ; consumed/editing/signal: retry
        jrst    native_sys_getchar_policy
        jrst    %L65
native_sys_getchar_buffered:
        sub     17,[1,,1]
        jrst    %L65
native_sys_getchar_error:
        sub     17,[1,,1]
        jrst    %L65

        ; MINIT first supplies the CTY bootstrap target and later retargets
        ; this word to the generic logical-TTY input dispatcher.
        .globl  native_sys_getchar_call
native_sys_getchar_call:
        jrst    pdp10_ret_neg1

native_sys_putchar:
        ; Bind the byte to the caller's controlling logical TTY before the
        ; MRES dispatcher selects CTY, DCS, or GE.
        pushj   17,proc_tty_output
        jumpl   1,pdp10_ret_neg1
        .globl  native_sys_putchar_call
native_sys_putchar_call:
        jrst    pdp10_ret_neg1
%L136:
        halt    .
%L137:
        seto    1,
; Leave the native syscall result in AC1 for mach_syscall.
%L65:
        popj    17,
