; syscall_dispatch.s -- compiler-derived native syscall dispatcher.
; Pointer checks and ABI movement are copied from known-good GCC output.
	.text
        .globl  pdp10_ret_zero
sys_user_words:
        hrrz    1,1
        hrrz    4,proc_table+3
        hlrz    3,proc_table+3
        add     3,4
        camge   1,4
        jrst    sys_user_words_bad
        caml    1,3
        jrst    sys_user_words_bad
        popj    17,
sys_user_words_bad:
        jrst    pdp10_ret_zero

	.globl	exec_native_syscall
	.globl	mach_syscall_ac2
	.globl	mach_syscall_ac3
	.globl	mach_syscall_ac4
	.globl	mach_syscall_ac5
exec_native_syscall:
	; AC1 is still the syscall number on entry from mach_syscall.
	hrrz 4,1
	subi 4,2
	jumpl 4,%L137
	caige 4,035                 ; syscall 31 - 2
	jrst exec_native_low
	subi 4,035
	caile 4,015                 ; syscalls 31..44
	jrst %L137
	jrst @exec_native_high(4)
exec_native_low:
	caile 4,024                 ; syscalls 2..22
	jrst %L137
	jrst @%L138(4)
%L138:
	.word	.66
	.word	.67
	.word	.137
	.word	.72
	.word	.73
	.word	.74
	.word	.75
	.word	.80
	.word	.90
	.word	.137
	.word	.137
	.word	.137
	.word	.97
	.word	.137
	.word	.137
	.word	.137
	.word	.102
	.word	.137
	.word	.107
	.word	.112
	.word	.119
exec_native_high:
	.word	.83
	.word	.86
	.word	.124
	.word	.129
	.word	.134
	.word	.135
	.word	.136
	.word	native_sys_chmod
	.word	native_sys_dtfs_format
	.word	native_sys_dtfs_mount
	.word	native_sys_unmount
	.word	native_sys_flock
	.word	native_sys_dup
	.word	native_sys_symlink
%L66:
	pushj 17,file_close_all
	pushj 17,mach_return_to_kernel_request
	hrrz 1,mach_syscall_ac2
	jrst %L65
%L67:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	hrrz 3,mach_syscall_ac3
	move 4,3
	andi 4,3
	addi 4,1
	caile 4,3
	movei 4,3
	andi 3,034
	ior 3,4
	move 2,3
	jrst file_open
%L72:
	hrrz 1,mach_syscall_ac2
	jrst file_close
%L73:
	move 1,mach_syscall_ac2
	andi 1,0177
	jrst native_sys_putchar
%L74:
	jrst native_sys_getchar
%L75:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	jrst file_chdir
%L80:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	hrrz 2,mach_syscall_ac3
	jrst file_getcwd
%L83:
	move 1,mach_syscall_ac3
	pushj 17,sys_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2
	hrrz 3,mach_syscall_ac4
	jrst file_read_words
%L86:
	move 1,mach_syscall_ac3
	pushj 17,sys_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2
	hrrz 3,mach_syscall_ac4
	move 4,mach_syscall_ac5
	jrst file_write_words
%L90:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	move 5,1
	move 1,mach_syscall_ac3
	pushj 17,sys_user_words
	move 3,1
	jumpe 5,%L137
	jumpe 1,%L137
	move 2,1
	move 1,5
	jrst file_stat_path
%L97:
	move 1,mach_syscall_ac3
	pushj 17,sys_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2
	jrst file_readdir
%L102:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	hrrz 2,mach_syscall_ac3
	jrst file_mkdir
%L107:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	jrst file_unlink
%L112:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	move 5,1
	move 1,mach_syscall_ac3
	pushj 17,sys_user_words
	move 3,1
	jumpe 5,%L137
	jumpe 1,%L137
	move 2,1
	move 1,5
	jrst file_rename
%L119:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	move 2,mach_syscall_ac3
	jrst file_truncate
%L124:
	move 1,mach_syscall_ac3
	pushj 17,sys_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2
	jrst sys_procinfo
%L129:
	move 1,mach_syscall_ac2
	pushj 17,sys_user_words
	jumpe 1,%L137
	jrst sys_meminfo
%L134:
	hrrz 1,mach_syscall_ac2
	jumpe 1,native_sys_getchar
	pushj 17,file_readchar
	camn 1,[-3]
	jrst native_sys_getchar
	jrst %L65
%L135:
	hrrz 1,mach_syscall_ac2
	move 2,mach_syscall_ac3
	andi 2,0777
	cail 1,1
	cail 1,3
	trna
	jrst native_sys_writechar_tty
	pushj 17,file_writechar
	came 1,[-3]
	jrst %L65
native_sys_writechar_tty:
	move 1,mach_syscall_ac3
	andi 1,0777
	jrst native_sys_putchar


; Return the DTC0 vnode for a valid user path, or zero on failure.
native_sys_dtc0_path:
        pushj 17,sys_user_words
        jumpe 1,native_sys_dtc0_path_fail
        movei 2,mach_syscall_ac5
        pushj 17,file_lookup_path
        jumpn 1,native_sys_dtc0_path_fail
        move 1,mach_syscall_ac5
        came 1,[020003000014]           ; DEVICEFS DTC0 directory
        jrst native_sys_dtc0_path_fail
        popj 17,
native_sys_dtc0_path_fail:
        setz 1,
        popj 17,

native_sys_chmod:
        move 1,mach_syscall_ac2
        pushj 17,sys_user_words
        jumpe 1,%L137
        movei 2,mach_syscall_ac5
        pushj 17,file_lookup_path
        jumpn 1,%L137
        move 1,mach_syscall_ac5
        hrrz 2,mach_syscall_ac3
        jrst vfs_chmod

native_sys_dtfs_format:
        move 1,mach_syscall_ac2
        pushj 17,native_sys_dtc0_path
        jumpe 1,%L137
        hrrz 2,mach_syscall_ac3         ; DTFS management control word
        move 3,2
        andi 3,07
        caile 3,1
        jrst %L137
        movei 1,0                       ; DTC0 unit
        .globl sys_dtfs_format_jump
sys_dtfs_format_jump:
        jrst pdp10_ret_neg1

native_sys_dtfs_mount:
        move 1,mach_syscall_ac2
        pushj 17,native_sys_dtc0_path
        jumpe 1,%L137
        move 1,mach_syscall_ac3
        pushj 17,sys_user_words
        jumpe 1,%L137
        movei 2,mach_syscall_ac5
        pushj 17,file_lookup_path
        jumpn 1,%L137
        hrrz 3,mach_syscall_ac4
        caile 3,1
        jrst %L137
        move 2,mach_syscall_ac5
        movei 1,0                       ; DTC0 unit
        movei 4,mach_syscall_ac5     ; returned root is not otherwise needed
        .globl sys_dtfs_mount_jump
sys_dtfs_mount_jump:
        jrst pdp10_ret_neg1

native_sys_unmount:
        move 1,mach_syscall_ac2
        pushj 17,sys_user_words
        jumpe 1,%L137
        movei 2,mach_syscall_ac5
        pushj 17,file_lookup_path
        jumpn 1,%L137
        move 1,mach_syscall_ac5
        jrst vfs_unmount

native_sys_flock:
        hrrz 1,mach_syscall_ac2
        hrrz 2,mach_syscall_ac3
        jrst file_lock

native_sys_dup:
        hrrz 1,mach_syscall_ac2
        jrst file_dup

native_sys_symlink:
        move 1,mach_syscall_ac2
        pushj 17,sys_user_words
        move 5,1
        move 1,mach_syscall_ac3
        pushj 17,sys_user_words
        jumpe 5,%L137
        jumpe 1,%L137
        move 2,1
        move 1,5
        jrst file_symlink

native_sys_getchar:
        seto    1,
        .globl  native_sys_getchar_call
native_sys_getchar_call:
        pushj   17,pdp10_ret_neg1
        jrst    %L65

native_sys_putchar:
        .globl  native_sys_putchar_call
native_sys_putchar_call:
        pushj   17,pdp10_ret_neg1
        jrst    %L65
%L136:
	pushj 17,pdp10_halt
	seto 1,
	jrst %L65
%L137:
	seto 1,
; Leave the native syscall result in AC1 for mach_syscall.
%L65:
%L60:
	popj 17,
