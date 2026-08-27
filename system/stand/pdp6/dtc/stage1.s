; stage1.s -- minimal opaque-image Stage1 for PDP-6 DECtape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 starts DECtape unit 0
; reading forward through the Type 136 data control.  The DECtape stream is:
;
;       word 0      DAIMON magic (ignored here)
;       word 1      image_words,,entry_offset (only image_words is needed)
;       remainder   opaque boot image -> 040000...
;
; The entry point is fixed at 040000.  Physical DECtape block boundaries are
; handled by the controller; Stage1 never buffers or restarts per block.
;
; Once the helper is installed, controller/media failures are reported as
; "?RDERR" and premature end-of-tape as "?BADTP".  Before that helper is
; complete, errors can only be halted safely.

        .text
        .globl start
        .globl __start

__start:
start:
        setom 000040
        setom 000041
        movei 017,070000
        pushj 017,install_bootstrap_sixbit

        ; DCT0: device 1 (DTC), device -> processor, move enabled.
        cono 0200,004040

        ; DTC0: selected, start forward, READ DATA.
        cono 0210,0220300

        ; The opaque stream header supplies the image word count. Its magic and entry
        ; offset are not needed by this fixed-entry Stage1.
        pushj 017,read_word
        pushj 017,read_word
        hlrz 02,03
        jumpe 02,bad_tape
        caile 02,020000
        jrst bad_tape

        movei 01,040000
load_loop:
        pushj 017,read_word
        movem 03,0(01)
        aoj 01,
        sojg 02,load_loop
        jrst 040000


; Return the next 36-bit DCT word in AC3.  DTC status B distinguishes genuine
; controller/data errors from reaching the end zone before the declared image
; length has been satisfied.
read_word:
        conso 0200,001000
        jrst read_wait
        datai 0200,03
        popj 017,

read_wait:
        consz 0214,0000034
        jrst read_error
        consz 0214,0000002
        jrst bad_tape
        jrst read_word

read_error:
        move 01,msg_rderr
        jrst diag

bad_tape:
        move 01,msg_badtp

diag:
        pushj 017,077760
        halt .
        jrst diag

msg_rderr: .word 0376244456262
msg_badtp: .word 0374241446460
        .include "../common/bootstrap-sixbit-077760.inc"
