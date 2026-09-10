; Common PDP-6 CTY SIXBIT word output.
;
; Entry:
;   AC1 = one packed SIXBIT word (six characters)
;   AC17 = pushdown pointer
;
; Clobbers AC2, AC3, AC4, AC6.

put_sixbit_word:
        move 02,01
        movei 06,06
put_six_loop:
        move 03,02
        lsh 03,-036
        andi 03,077
        addi 03,040
put_six_wait:
        coni 0120,04
        trne 04,0020
        jrst put_six_wait
        datao 0120,03
        lsh 02,06
        sojg 06,put_six_loop
put_six_final_wait:
        coni 0120,04
        trne 04,0020
        jrst put_six_final_wait
        popj 017,
