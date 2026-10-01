; PDP-6/PDP-10 foreign DTFS chain walker.
        .text
        .globl dtfs_chain_walk
dtfs_chain_walk:
	add 017,[016,,016]
	movem 016,-015(017)
	movei 0,-014(017)
	hrli 0,010
	blt 0,-7(017)
	move 015,1
	move 012,3
	move 016,4
	addi 2,1
	movem 2,-2(017)
	setzm (017)
	move 6,-020(017)
	caie 6,056
	jrst dtfs_cw_320
	setom (017)
	movei 014,0
	movei 013,1
	movei 6,01070
	skipn -021(017)
	addi 6,7
	movem 6,-1(017)
	jrst dtfs_cw_323
dtfs_cw_320:
	setzm -6(017)
	setzm -5(017)
	movei 013,1
	move 010,013
	sub 010,-020(017)
dtfs_cw_331:
	movei 1,0
	move 2,010
	pushj 17,dtfs_owner
	came 1,-2(017)
	jrst dtfs_cw_326
	aos -6(017)
	skipe -5(017)
	jrst dtfs_cw_326
	move 1,015
	move 2,013
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_cw_326
	ldb 4,[POINT 10,fs_block_workspace,27]
	camn 4,013
	movem 013,-5(017)
dtfs_cw_326:
	addi 010,1
	addi 013,1
	caie 013,01102
	jrst dtfs_cw_331
	move 013,-5(017)
	setz 014,
	setzm -4(017)
	jumpn 013,dtfs_cw_336
	move 1,-6(017)
	ior 1,-020(017)
	jumpe 1,dtfs_cw_319
	jrst dtfs_cw_378
dtfs_cw_336:
	cail 013,1
	cail 013,01102
	jrst dtfs_cw_374
	move 2,013
	sub 2,-020(017)
	movei 1,0
	pushj 17,dtfs_owner
	came 1,-2(017)
	jrst dtfs_cw_374
	move 1,015
	move 2,013
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_cw_374
	ldb 4,[POINT 10,fs_block_workspace,27]
	came 4,-5(017)
	jrst dtfs_cw_374
	move 010,fs_block_workspace
	andi 010,0377
	caile 010,0177
	jrst dtfs_cw_374
	jumpn 016,dtfs_cw_native_transfer
	add 014,010
	jrst dtfs_cw_342
dtfs_cw_native_transfer:
	movei 6,1
	jrst dtfs_cw_343
dtfs_cw_323:
	came 013,-1(017)
	jrst dtfs_cw_344
	move 1,014
	skipn -021(017)
	jrst dtfs_cw_319
dtfs_cw_378:
	seto 1,
	jrst dtfs_cw_319
dtfs_cw_344:
	move 2,013
	subi 2,1
	movei 1,056
	pushj 17,dtfs_owner
	move 010,1
	skipe -021(017)
	jrst dtfs_cw_347
	cain 1,037
	jrst dtfs_cw_done
dtfs_cw_347:
	came 010,-2(017)
	jrst dtfs_cw_377
	movei 010,0200
	jumpl 012,dtfs_cw_its_skip
	caige 012,0200
	jrst dtfs_cw_349
dtfs_cw_its_skip:
	subi 012,0200
dtfs_cw_377:
	aoja 013,dtfs_cw_323
dtfs_cw_349:
	move 1,015
	move 2,013
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_cw_374
	movei 6,0
dtfs_cw_343:
	jumpl 012,dtfs_cw_352
	caml 012,010
	jrst dtfs_cw_352
	move 011,010
	sub 011,012
	move 4,-017(017)
	sub 4,014
	jumpl 4,dtfs_cw_take_ok
	camle 011,4
	move 011,4
dtfs_cw_take_ok:
	skipn -021(017)
	jrst dtfs_cw_354
	move 1,016
	add 1,014
	move 2,6
	add 2,012
	movei 2,fs_block_workspace(2)
	move 3,011
	pushj 17,fs_copy_words
	move 1,015
	move 2,013
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_write
	jumpn 1,dtfs_cw_374
	jrst dtfs_cw_361
dtfs_cw_354:
	move 1,6
	add 1,012
	movei 1,fs_block_workspace(1)
	move 2,016
	add 2,014
	move 3,011
	pushj 17,fs_copy_words
dtfs_cw_361:
	add 014,011
	movei 012,0
	camn 014,-017(017)
	jrst dtfs_cw_done
	jrst dtfs_cw_367
dtfs_cw_352:
	sub 012,010
dtfs_cw_367:
	addi 013,1
	skipe (017)
	jrst dtfs_cw_323
dtfs_cw_342:
	ldb 013,[POINT 10,fs_block_workspace,17]
	aos -4(017)
	jumpn 013,dtfs_cw_369
	move 6,-4(017)
	came 6,-6(017)
	jrst dtfs_cw_374
	move 1,014
	jrst dtfs_cw_319
dtfs_cw_done:
	move 1,014
	jrst dtfs_cw_319
dtfs_cw_369:
	jumpe 010,dtfs_cw_374
	move 7,-4(017)
	came 7,-6(017)
	jrst dtfs_cw_336
dtfs_cw_374:
	seto 1,
dtfs_cw_319:
	move 016,-015(017)
	movei 0,010
	hrli 0,-014(017)
	blt 0,015
	sub 017,[016,,016]
	popj 17,


; Foreign DTFS allocation-map census / first-block finder.
        .globl dtfs_block_info
dtfs_block_info:
	add 017,[6,,6]
	movei 0,-5(017)
	hrli 0,010
	blt 0,(017)
	movei 013,1(2)
	move 011,3
	movei 014,0
	jumpe 3,dtfs_bi_52
	setzm (3)
dtfs_bi_52:
	ldb 4,[POINT 6,1,11]
	move 4,dtfs_media-1(4)
	move 012,4
	lsh 012,-3
	move 015,4
	andi 015,7
	movei 010,1
	andcai 012,1
	trnn 4,020
	jrst dtfs_bi_70
	movei 010,0
dtfs_bi_59:
	movei 1,056
	move 2,010
	pushj 17,dtfs_owner
	camn 1,013
	addi 014,1
	addi 010,1
	caie 010,01076
	jrst dtfs_bi_59
	move 1,014
	jrst dtfs_bi_51
dtfs_bi_70:
	movei 1,0
	move 2,012
	pushj 17,dtfs_owner
	came 1,013
	jrst dtfs_bi_62
	addi 014,1
	jumpe 011,dtfs_bi_62
	skipe (011)
	jrst dtfs_bi_62
	move 1,015
	move 2,010
	movei 3,fs_block_workspace
	pushj 17,dtfs_dtc_read
	jumpn 1,dtfs_bi_zero
	ldb 4,[POINT 10,fs_block_workspace,27]
	camn 4,010
	movem 010,(011)
dtfs_bi_62:
	addi 012,1
	addi 010,1
	caie 010,01102
	jrst dtfs_bi_70
	move 1,014
	jrst dtfs_bi_51
dtfs_bi_zero:
	setz 1,
dtfs_bi_51:
	movei 0,010
	hrli 0,-5(017)
	blt 0,015
	sub 017,[6,,6]
	popj 17,
