/**
 * @file sixbit-out.inc
 * @brief Relocatable PDP-6 CTY SIXBIT-word output helper.
 *
 * Entry AC1 is one packed six-character SIXBIT word and AC17 is the pushdown
 * pointer. AC2/AC3/AC4/AC6 are clobbered. CTY device 0120 is polled and the
 * routine waits for the final character to leave the interface before return.
 */
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
