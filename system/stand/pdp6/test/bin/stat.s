; stat.s -- opaque PDP-6 Stage1 test payload.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 017,050000
        movem 02,dsk_count
        move 01,msg_boot0
        pushj 017,put_sixbit_word
        move 01,msg_boot1
        pushj 017,put_sixbit_word
        move 01,msg_boot2
        pushj 017,put_sixbit_word
        pushj 017,put_crlf
        pushj 017,put_dsk_reads
        move 01,msg_done0
        pushj 017,put_sixbit_word
        move 01,msg_done1
        pushj 017,put_sixbit_word
        pushj 017,put_crlf
        halt .
        jrst .

put_dsk_reads:
        move 05,dsk_count
        jumpe 05,put_dsk_done
        setzm dsk_index
put_dsk_loop:
        move 04,dsk_index
        caml 04,dsk_count
        jrst put_dsk_done
        move 01,msg_dsk0(04)
        pushj 017,put_sixbit_word
        move 01,msg_read
        pushj 017,put_sixbit_word
        pushj 017,put_crlf
        aos dsk_index
        jrst put_dsk_loop
put_dsk_done:
        popj 017,

put_crlf:
        movei 01,015
        pushj 017,putc
        movei 01,012
        pushj 017,putc
        popj 017,

put_sixbit_word:
        movem 01,put_word
        movei 06,0
put_six_loop:
        caige 06,06
        jrst put_six_one
        popj 017,
put_six_one:
        move 02,put_word
        move 03,put_shift(06)
        lsh 02,0(03)
        andi 02,077
        addi 02,040
        move 01,02
        pushj 017,putc
        aoj 06,
        jrst put_six_loop

putc:
        movem 01,ioword
putc_wait:
        coni 0120,cty_status
        move 02,cty_status
        trne 02,0020
        jrst putc_wait
        datao 0120,ioword
        popj 017,

put_shift: .word -036
           .word -030
           .word -022
           .word -014
           .word -06
           .word 0
put_word:  .word 0
ioword:    .word 0
cty_status:.word 0
dsk_count: .word 0
dsk_index: .word 0
msg_boot0: .word 0425757645457
msg_boot1: .word 0414445620057
msg_boot2: .word 0530000000000
msg_dsk0:  .word 0446353200000
           .word 0446353210000
           .word 0446353220000
           .word 0446353230000
msg_read:  .word 0624541440000
msg_done0: .word 0644563640000
msg_done1: .word 0445756450000
