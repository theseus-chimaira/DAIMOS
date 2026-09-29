; PDP-10 target implementation of DTFS resize.
; Initially derived mechanically from GCC -Os output; keep host C as reference.

.globl  dtfs_resize
dtfs_resize:
	add 017,[015,,015]
	movem 016,-014(017)
	movei 0,-013(017)
	hrli 0,010
	blt 0,-6(017)
	setzb 010,011
	movem 1,-3(017)
	move 014,2
	pushj 17,dtfs_is_file
	jumpe 1,dtfs_r_169
	move 1,-3(017)
	pushj 17,dtfs_load
	jumpn 1,dtfs_r_169
	seto 3,
	cail 014,0
	cail 014,0217100
	jrst dtfs_r_80
	move 1,-3(017)
	pushj 17,dtfs_personality
	.if DTFS_ENABLE_ITS
	caie 1,020
	jrst dtfs_r_84
	move 1,-3(017)
	move 2,014
	movei 3,0
	pushj 17,dtfs_its_resize
	jrst dtfs_r_170
	.endif
dtfs_r_84:
	movei 6,010
	came 1,6
	tdza 6,6
	movei 6,1
	movem 6,(017)
	move 6,-3(017)
	hrrzm 6,-2(017)
	move 1,6
	pushj 17,dtfs_unit
	move 015,1
	move 6,-2(017)
	addi 6,1
	movem 6,-1(017)
	move 1,-3(017)
	move 2,-2(017)
	movei 3,-5(017)
	pushj 17,dtfs_block_info
	move 012,1
	jumpe 014,dtfs_r_87
	move 010,014
	subi 010,1
	idivi 010,0177
	move 013,010
	addi 013,1
	move 016,011
	addi 016,1
	jrst dtfs_r_90
dtfs_r_87:
	move 013,(017)
dtfs_r_88:
	movei 016,0
dtfs_r_90:
	jumpn 012,dtfs_r_91
	movei 4,-5(017)
	setzb 014,(4)
	jrst dtfs_r_92
dtfs_r_91:
	movei 4,-5(017)
	skipn 2,(4)
	jrst dtfs_r_169
	move 014,2
	movei 011,1
	cain 012,1
	jrst dtfs_r_92
dtfs_r_102:
	move 1,015
	move 2,014
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_r_169
	ldb 010,[POINT 10,fs_block_workspace,17]
	cail 010,1
	cail 010,01102
	jrst dtfs_r_169
	move 014,010
	addi 011,1
	came 011,012
	jrst dtfs_r_102
dtfs_r_92:
	caml 012,013
	jrst dtfs_r_103
dtfs_r_104:
	move 1,014
	addi 1,1
	move 2,(017)
	movei 3,-4(017)
	pushj 17,dtfs_find_free_block
	jumpn 1,dtfs_r_169
	movei 010,-5(017)
	skipe (010)
	jrst dtfs_r_110
	movei 4,-4(017)
	move 4,(4)
	movem 4,(010)
dtfs_r_110:
	pushj 17,fs_zero_block_workspace
	move 011,010
	move 4,(010)
	lsh 4,010
	addi 012,1
	move 3,4
	ior 3,016
	camn 012,013
	jrst dtfs_r_112
	move 3,4
	iori 3,0177
dtfs_r_112:
	movem 3,fs_block_workspace
	move 1,015
	movei 010,-4(017)
	move 2,(010)
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_write
	jumpn 1,dtfs_r_169
	move 2,(010)
	sub 2,(017)
	move 3,-1(017)
	pushj 17,dtfs_set_owner
	jumpe 014,dtfs_r_116
	move 1,015
	move 2,014
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_r_169
	hrlz 4,(010)
	move 3,(011)
	lsh 3,010
	ior 4,3
	iori 4,0177
	movem 4,fs_block_workspace
	move 1,015
	move 2,014
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_write
	jumpn 1,dtfs_r_169
dtfs_r_116:
	move 014,(010)
	came 012,013
	jrst dtfs_r_104
	jrst dtfs_r_124
dtfs_r_103:
	caml 013,012
	jrst dtfs_r_125
	jumpn 013,dtfs_r_126
	movei 3,-5(017)
	movei 4,-4(017)
	move 3,(3)
	movem 3,(4)
	jrst dtfs_r_127
dtfs_r_126:
	movei 012,-5(017)
	movei 4,-4(017)
	move 6,(012)
	movem 6,(4)
	movei 011,1
	cain 013,1
	jrst dtfs_r_161
dtfs_r_135:
	movei 010,-4(017)
	move 2,(010)
	move 1,015
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_r_169
	ldb 4,[POINT 10,fs_block_workspace,17]
	movem 4,(010)
	addi 011,1
	came 011,013
	jrst dtfs_r_135
dtfs_r_161:
	move 1,015
	movei 011,-4(017)
	move 2,(011)
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_r_169
	ldb 010,[POINT 10,fs_block_workspace,17]
	move 4,(012)
	lsh 4,010
	ior 4,016
	movem 4,fs_block_workspace
	move 1,015
	move 2,(011)
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_write
	jumpn 1,dtfs_r_169
	movem 010,(011)
dtfs_r_127:
	movei 4,-4(017)
	skipn (4)
	jrst dtfs_r_124
dtfs_r_149:
	movei 011,-4(017)
	move 2,(011)
	cail 2,0
	cail 2,01102
	jrst dtfs_r_169
	move 1,015
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_r_169
	ldb 010,[POINT 10,fs_block_workspace,17]
	move 2,(011)
	sub 2,(017)
	movei 3,0
	pushj 17,dtfs_set_owner
	movem 010,(011)
	jumpn 010,dtfs_r_149
	jrst dtfs_r_124
dtfs_r_125:
	jumpe 013,dtfs_r_124
	move 1,015
	move 2,014
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_r_169
	movei 4,-5(017)
	move 4,(4)
	lsh 4,010
	ior 4,016
	movem 4,fs_block_workspace
	move 1,015
	move 2,014
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_write
	jumpn 1,dtfs_r_169
	jrst dtfs_r_124
dtfs_r_169:
	seto 3,
	jrst dtfs_r_80
dtfs_r_124:
	skipe (017)
	jrst dtfs_r_158
	move 1,-2(017)
	move 2,016
	pushj 17,dtfs_set_last_words
dtfs_r_158:
	move 1,-3(017)
	pushj 17,dtfs_commit
dtfs_r_170:
	move 3,1
dtfs_r_80:
	move 1,3
	move 016,-014(017)
	movei 0,010
	hrli 0,-013(017)
	blt 0,015
	sub 017,[015,,015]
	popj 17,
