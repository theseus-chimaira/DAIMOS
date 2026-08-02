; paper_stage1.s -- sequential paper-tape DAIMON Stage1 for PDP-6.
;
; The tape is five bytes per 36-bit word, most significant nibble first.
; Stage1 interprets only the three-word DAIMON header, places KCORE at 000040,
; places the opaque initialization stream ending at 0770000, constructs a
; paper-source BOOTINFO, and enters the normal KINIT entry.

        .text
        .globl start
        .globl __start

__start:
start:
        movei 17,050000
        movei 1,0020
        cono 0104,0(1)
        movei 1,header
        movei 2,3
        pushj 17,read_words
        move 2,header
        camn 2,daimon_magic
        jrst header_ok
        jrst fail
header_ok:
        hlrz 2,header+1
        movem 2,kcore_words
        hrrz 3,header+1
        movem 3,init_words
        hlrz 4,header+2
        movem 4,entry_off
        movei 5,0770000
        sub 5,3
        movem 5,init_base
        movei 6,000040
        add 6,2
        movem 6,kcore_end
        move 7,5
        sub 7,6
        caige 7,000200
        jrst fail

        movei 1,000040
        move 2,kcore_words
        pushj 17,read_words
        move 1,init_base
        move 2,init_words
        pushj 17,read_words
        move 2,paper_cold_words_value
        movem 2,cold_words
        move 1,init_base
        sub 1,2
        camg 1,kcore_end
        jrst fail
        movem 1,cold_base
        pushj 17,read_words
        pushj 17,build_bootinfo

        move 2,init_base
        add 2,entry_off
        movem 2,entry_addr
        movei 17,050000
        movei 1,073000
        jrst @entry_addr

read_words:
        jumpe 2,read_done
read_loop:
        push 17,1
        pushj 17,read_word
        pop 17,1
        movem 3,0(1)
        aoj 1,
        sojg 2,read_loop
read_done:
        popj 17,

read_word:
        setz 3,
        movei 4,5
read_byte_loop:
        pushj 17,ptr_getc
        lsh 3,8
        ior 3,1
        sojg 4,read_byte_loop
        popj 17,

ptr_getc:
ptr_wait:
        coni 0104,tmp
        move 1,tmp
        trnn 1,0010
        jrst ptr_wait
        datai 0104,ioword
        move 1,ioword
        andi 1,0377
        popj 17,

build_bootinfo:
        movei 1,073000
        movei 2,000035
zero_loop:
        setzm 0(1)
        aoj 1,
        sojg 2,zero_loop
        move 2,bi2_header
        movem 2,073000
        movei 2,000035
        movem 2,073001
        movei 2,1
        movem 2,073002             ; BOOTINFO_F_PAPER_SOURCE
        movei 2,000040
        movem 2,073023
        move 2,kcore_words
        movem 2,073024
        movem 2,073025
        move 2,init_base
        add 2,entry_off
        movem 2,073026
        move 2,init_base
        movem 2,073027
        move 2,init_words
        movem 2,073030
        movem 2,073031
        move 2,entry_off
        movem 2,073032
        move 2,cold_base
        movem 2,073033
        move 2,cold_words
        movem 2,073034
        popj 17,

fail:
        halt .
        jrst fail

header:      .block 3
kcore_words: .word 0
init_words:  .word 0
entry_off:   .word 0
init_base:   .word 0
entry_addr:  .word 0
kcore_end:  .word 0
cold_base:  .word 0
cold_words: .word 0
ioword:      .word 0
tmp:         .word 0

daimon_magic: .word 0444151555756
bi2_header:   .word 0425122020000

paper_cold_words_value: .word 0
