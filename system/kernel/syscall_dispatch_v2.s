; syscall_dispatch_v2.s -- compiler-derived native syscall dispatcher.
; Pointer checks and ABI movement are copied from known-good GCC output.
	.text
sys_v1_user_words:
	move 2,1
	movei 1,0
	skipn 3,proc_v1_current
	jrst %L55
	hrrz 4,1(3)
	hlrz 3,1(3)
	add 3,4
	hrrz 1,2
	move 2,1
	tlc 2,0400000
	tlc 4,0400000
	camge 2,4
	jrst %L58
	move 4,3
	tlc 4,0400000
	caml 2,4
%L58:
	movei 1,0
%L55:
	popj 17,

	.globl	exec_native_syscall_v1
exec_native_syscall_v1:
	add 017,[3,,3]
	movem 010,-2(017)
	movem 011,-1(017)
	move 011,1
	jumpe 1,%L62
	skipn proc_v1_current
	jrst %L62
	jrst %L61
%L62:
	seto 1,
	jrst %L60
%L61:
	hrrz 4,(011)
	subi 4,2
	jumpl 4,%L137
	caile 4,043
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
%L66:
	move 1,proc_v1_current
	movei 2,4
	dpb 2,[POINT 3,(1),20]
	pushj 17,mach_return_to_kernel_request_v1
	hrrz 1,1(011)
	jrst %L65
%L67:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	hrrz 3,2(011)
	move 4,3
	andi 4,3
	addi 4,1
	caile 4,3
	movei 4,3
	andi 3,034
	ior 3,4
	move 2,3
	pushj 17,file_v1_open
	jrst %L65
%L72:
	hrrz 1,1(011)
	pushj 17,file_v1_close
	jrst %L65
%L73:
	move 1,1(011)
	andi 1,0177
	jrst native_sys_putchar
%L74:
	jrst native_sys_getchar
%L75:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	pushj 17,file_v1_chdir
	jrst %L65
%L80:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	hrrz 2,2(011)
	pushj 17,file_v1_getcwd
	jrst %L65
%L83:
	move 1,2(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,1(011)
	hrrz 3,3(011)
	pushj 17,file_v1_read_words
	jrst %L65
%L86:
	move 1,2(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,1(011)
	hrrz 3,3(011)
	move 4,4(011)
	pushj 17,file_v1_write_words
	jrst %L65
%L90:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	move 010,1
	move 1,2(011)
	pushj 17,sys_v1_user_words
	move 3,1
	jumpe 010,%L137
	jumpe 1,%L137
	move 2,1
	move 1,010
	pushj 17,file_v1_stat_path
	jrst %L65
%L97:
	move 1,2(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,1(011)
	pushj 17,file_v1_readdir
	jrst %L65
%L102:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	hrrz 2,2(011)
	pushj 17,file_v1_mkdir
	jrst %L65
%L107:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	pushj 17,file_v1_unlink
	jrst %L65
%L112:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	move 010,1
	move 1,2(011)
	pushj 17,sys_v1_user_words
	move 3,1
	jumpe 010,%L137
	jumpe 1,%L137
	move 2,1
	move 1,010
	pushj 17,file_v1_rename
	jrst %L65
%L119:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,2(011)
	pushj 17,file_v1_truncate
	jrst %L65
%L124:
	move 1,2(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	move 2,1
	hrrz 1,1(011)
	pushj 17,sys_v1_procinfo
	jrst %L65
%L129:
	move 1,1(011)
	pushj 17,sys_v1_user_words
	jumpe 1,%L137
	pushj 17,sys_v1_meminfo
	jrst %L65
%L134:
	hrrz 1,1(011)
	jumpe 1,native_sys_getchar
	pushj 17,file_v1_readchar
	camn 1,[-3]
	jrst native_sys_getchar
	jrst %L65
%L135:
	hrrz 1,1(011)
	move 2,2(011)
	andi 2,0777
	move 010,2
	cail 1,1
	cail 1,3
	trna
	jrst native_sys_writechar_tty
	pushj 17,file_v1_writechar
	came 1,[-3]
	jrst %L65
native_sys_writechar_tty:
	move 1,010
	jrst native_sys_putchar

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
	move 010,-2(017)
	move 011,-1(017)
	sub 017,[3,,3]
	popj 17,
