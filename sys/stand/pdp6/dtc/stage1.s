; dectape_boot_stub.s -- standalone PDP-6 DECtape boot placeholder.
;
; This intentionally does not read DTC551 media or enter KINIT.  It proves the
; build/launch slot for a future block-addressed DECtape Stage1 and leaves the
; processor halted at dectape_stub_halt after announcing itself on CTY0.

        .text
        .globl start
        .globl __start
        .globl dectape_stub_halt

__start:
start:
        movei 17,040000
        move 1,msg_dta
        pushj 17,put_sixbit_word
        move 1,msg_boot
        pushj 17,put_sixbit_word
        move 1,msg_stub
        pushj 17,put_sixbit_word
        move 1,msg_ready
        pushj 17,put_sixbit_word
        movei 1,15
        pushj 17,putc
        movei 1,12
        pushj 17,putc
dectape_stub_halt:
        halt .
        jrst dectape_stub_halt

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
msg_dta:   .word 0446441000000
msg_boot:  .word 0425757640000
msg_stub:  .word 0636465420000
msg_ready: .word 0624541447100
