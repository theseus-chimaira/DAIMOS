; stat.s -- standalone PDP-6 Stage1 status payload.
;
; Loaded at the normal KINIT address and entered with AC1 = BOOTINFO.
; This intentionally does not call kernel code.  It emits short CTY status
; records and halts.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 17,050000
        move 1,msg_stage
        pushj 17,put_sixbit_word
        move 1,msg_ok
        pushj 17,put_sixbit_word
        pushj 17,put_crlf
        pushj 17,put_dsk_reads
        halt .
        jrst .

put_dsk_reads:
        move 5,073010
        lsh 5,-17
        andi 5,037
        jumpe 5,put_dsk_done
        movem 5,dsk_count
        setzm dsk_index
put_dsk_loop:
        move 4,dsk_index
        caml 4,dsk_count
        jrst put_dsk_done
        move 1,msg_dsk0(4)
        pushj 17,put_sixbit_word
        move 1,msg_read
        pushj 17,put_sixbit_word
        pushj 17,put_crlf
        aos dsk_index
        jrst put_dsk_loop
put_dsk_done:
        popj 17,

put_crlf:
        movei 1,15
        pushj 17,putc
        movei 1,12
        pushj 17,putc
        popj 17,

put_sixbit_word:
        movem 1,put_word
        movei 6,0
put_six_loop:
        caige 6,6
        jrst put_six_one
        popj 17,
put_six_one:
        move 2,put_word
        move 3,put_shift(6)
        lsh 2,0(3)
        andi 2,077
        addi 2,040
        move 1,2
        pushj 17,putc
        aoj 6,
        jrst put_six_loop

putc:
        movem 1,ioword
putc_wait:
        coni 0120,cty_status
        move 2,cty_status
        trne 2,0020
        jrst putc_wait
        datao 0120,ioword
        popj 17,

put_shift: .word -36
           .word -30
           .word -22
           .word -14
           .word -6
           .word 0
put_word:  .word 0
ioword:    .word 0
cty_status:.word 0
dsk_count: .word 0
dsk_index: .word 0
msg_stage: .word 636441474521        ; "STAGE1"
msg_ok:    .word 005753000000        ; " OK   "
msg_dsk0:  .word 446353200000        ; "DSK0  "
           .word 446353210000        ; "DSK1  "
           .word 446353220000        ; "DSK2  "
           .word 446353230000        ; "DSK3  "
msg_read:  .word 624541440000        ; "READ  "
