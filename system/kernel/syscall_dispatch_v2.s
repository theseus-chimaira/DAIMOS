; syscall_dispatch_v2.s -- compiler-derived native syscall dispatcher.
; Pointer checks and ABI movement are copied from known-good GCC output.
	.text
        .globl  pdp10_ret_zero_v1
sys_v1_user_words:
        hrrz    1,1
        hrrz    4,proc_v1_table+3
        hlrz    3,proc_v1_table+3
        add     3,4
        camge   1,4
        jrst    sys_v1_user_words_bad
        caml    1,3
        jrst    sys_v1_user_words_bad
        popj    17,
sys_v1_user_words_bad:
        jrst    pdp10_ret_zero_v1

	.globl	exec_native_syscall_v1
	.globl	mach_syscall_ac1_v1
	.globl	mach_syscall_ac2_v1
	.globl	mach_syscall_ac3_v1
	.globl	mach_syscall_ac4_v1
	.globl	mach_syscall_ac5_v1
exec_native_syscall_v1:
	hrrz 4,mach_syscall_ac1_v1
	subi 4,2
	jumpl 4,%L137
	caile 4,052
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
	.word	.137
	.word	.137
	.word	.137
	.word	.137
	.word	.137
	.word	.137
	.word	.137
	.word	.137
	.word	.83
	.word	.86
	.word	.124
	.word	.129
	.word	.134
	.word	.135
	.word	.136
	.word	native_sys_chmod_v1
	.word	native_sys_dtfs_format_v1
	.word	native_sys_dtfs_mount_v1
	.word	native_sys_unmount_v1
	.word	native_sys_flock_v1
	.word	native_sys_dup_v1
	.word	native_sys_symlink_v1
%L66:
	pushj 17,file_close_all
	pushj 17,mach_return_to_kernel_request_v1
	hrrz 1,mach_syscall_ac2_v1
	jrst %L65
%L67:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	hrrz 3,mach_syscall_ac3_v1
	move 4,3
	andi 4,3
	addi 4,1
	caile 4,3
	movei 4,3
	andi 3,034
	ior 3,4
	move 2,3
	pushj 17,file_open
	jrst %L65
%L72:
	hrrz 1,mach_syscall_ac2_v1
	pushj 17,file_close
	jrst %L65
%L73:
	move 1,mach_syscall_ac2_v1
	andi 1,0177
	jrst native_sys_putchar
%L74:
	jrst native_sys_getchar
%L75:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	pushj 17,file_chdir
	jrst %L65
%L80:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	hrrz 2,mach_syscall_ac3_v1
	pushj 17,file_getcwd
	jrst %L65
%L83:
	move 1,mach_syscall_ac3_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2_v1
	hrrz 3,mach_syscall_ac4_v1
	pushj 17,file_read_words
	jrst %L65
%L86:
	move 1,mach_syscall_ac3_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2_v1
	hrrz 3,mach_syscall_ac4_v1
	move 4,mach_syscall_ac5_v1
	pushj 17,file_write_words
	jrst %L65
%L90:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	move 5,1
	move 1,mach_syscall_ac3_v1
	pushj 17,sys_v1_user_words
	move 3,1
	jumpe 5,%L137
	jumpe 1,%L137
	move 2,1
	move 1,5
	pushj 17,file_stat_path
	jrst %L65
%L97:
	move 1,mach_syscall_ac3_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2_v1
	pushj 17,file_readdir
	jrst %L65
%L102:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	hrrz 2,mach_syscall_ac3_v1
	pushj 17,file_mkdir
	jrst %L65
%L107:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	pushj 17,file_unlink
	jrst %L65
%L112:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	move 5,1
	move 1,mach_syscall_ac3_v1
	pushj 17,sys_v1_user_words
	move 3,1
	jumpe 5,%L137
	jumpe 1,%L137
	move 2,1
	move 1,5
	pushj 17,file_rename
	jrst %L65
%L119:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,mach_syscall_ac3_v1
	pushj 17,file_truncate
	jrst %L65
%L124:
	move 1,mach_syscall_ac3_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_procinfo
	jrst %L65
%L129:
	move 1,mach_syscall_ac2_v1
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	pushj 17,sys_v1_meminfo
	jrst %L65
%L134:
	hrrz 1,mach_syscall_ac2_v1
	jumpe 1,native_sys_getchar
	pushj 17,file_readchar
	camn 1,[-3]
	jrst native_sys_getchar
	jrst %L65
%L135:
	hrrz 1,mach_syscall_ac2_v1
	move 2,mach_syscall_ac3_v1
	andi 2,0777
	cail 1,1
	cail 1,3
	trna
	jrst native_sys_writechar_tty
	pushj 17,file_writechar
	came 1,[-3]
	jrst %L65
native_sys_writechar_tty:
	move 1,mach_syscall_ac3_v1
	andi 1,0777
	jrst native_sys_putchar


; Return the DTC0 vnode for a valid user path, or zero on failure.
native_sys_dtc0_path_v1:
        pushj 17,sys_v1_user_words
        jumpe 1,native_sys_dtc0_path_fail_v1
        movei 2,mach_syscall_ac5_v1
        pushj 17,file_lookup_path
        jumpn 1,native_sys_dtc0_path_fail_v1
        move 1,mach_syscall_ac5_v1
        came 1,[020002000014]           ; DEVICEFS DTC0
        jrst native_sys_dtc0_path_fail_v1
        popj 17,
native_sys_dtc0_path_fail_v1:
        setz 1,
        popj 17,

native_sys_chmod_v1:
        move 1,mach_syscall_ac2_v1
        pushj 17,sys_v1_user_words
        jumpe 1,%L137
        movei 2,mach_syscall_ac5_v1
        pushj 17,file_lookup_path
        jumpn 1,%L137
        move 1,mach_syscall_ac5_v1
        hrrz 2,mach_syscall_ac3_v1
        pushj 17,vfs_chmod
        jrst %L65

native_sys_dtfs_format_v1:
        move 1,mach_syscall_ac2_v1
        pushj 17,native_sys_dtc0_path_v1
        jumpe 1,%L137
        movei 1,0                       ; DTC0 unit
        pushj 17,fs_dtfs_format_unit
        jrst %L65

native_sys_dtfs_mount_v1:
        move 1,mach_syscall_ac2_v1
        pushj 17,native_sys_dtc0_path_v1
        jumpe 1,%L137
        move 1,mach_syscall_ac3_v1
        pushj 17,sys_v1_user_words
        jumpe 1,%L137
        movei 2,mach_syscall_ac5_v1
        pushj 17,file_lookup_path
        jumpn 1,%L137
        hrrz 3,mach_syscall_ac4_v1
        caile 3,1
        jrst %L137
        move 2,mach_syscall_ac5_v1
        movei 1,0                       ; DTC0 unit
        movei 4,mach_syscall_ac5_v1     ; returned root is not otherwise needed
        pushj 17,fs_dtfs_mount_unit
        jrst %L65

native_sys_unmount_v1:
        move 1,mach_syscall_ac2_v1
        pushj 17,sys_v1_user_words
        jumpe 1,%L137
        movei 2,mach_syscall_ac5_v1
        pushj 17,file_lookup_path
        jumpn 1,%L137
        move 1,mach_syscall_ac5_v1
        pushj 17,vfs_unmount
        jrst %L65

native_sys_flock_v1:
        hrrz 1,mach_syscall_ac2_v1
        hrrz 2,mach_syscall_ac3_v1
        pushj 17,file_lock
        jrst %L65

native_sys_dup_v1:
        hrrz 1,mach_syscall_ac2_v1
        pushj 17,file_dup
        jrst %L65

native_sys_symlink_v1:
        move 1,mach_syscall_ac2_v1
        pushj 17,sys_v1_user_words
        move 5,1
        move 1,mach_syscall_ac3_v1
        pushj 17,sys_v1_user_words
        jumpe 5,%L137
        jumpe 1,%L137
        move 2,1
        move 1,5
        pushj 17,file_symlink
        jrst %L65

native_sys_getchar:
	seto 1,
	skipe 4,kcore_cty_getchar_v1
	pushj 17,(4)
	jrst %L65

native_sys_putchar:
	move 5,1
	seto 1,
	skipn 4,kcore_cty_putchar_v1
	jrst %L65
	move 1,5
	pushj 17,(4)
	jrst %L65
%L136:
	pushj 17,pdp10_halt
	seto 1,
	jrst %L65
%L137:
	seto 1,
; Leave the native syscall result in AC1 for mach_syscall_v1.
%L65:
%L60:
	popj 17,
