; proc_pdp10.s -- process-table placement for PDP-6/PDP-10.
        .equ    proc_v1_table,0601142
        .globl  proc_v1_table

	.text
	.globl	proc_v1_get
proc_v1_get:
	; Preserve the C unsigned slot < 64 check, including high-bit values.
	cail	1,0
	cail	1,0100
	jrst	proc_v1_get_bad
	lsh	1,1
	movei	1,proc_v1_table(1)
	ldb	2,[POINT 3,(1),20]
	skipn	2
	movei	1,0
	popj	17,
proc_v1_get_bad:
	movei	1,0
	popj	17,
