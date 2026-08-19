; stage1.s -- minimal opaque-image Stage1 for PDP-6 DECtape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 starts DECtape unit 0
; reading forward through the Type 136 data control.  The existing DECtape
; stream begins with the two-word DAIMON header; only its image word count is
; needed here.  The entry point is fixed at 040000 and the image is otherwise
; opaque.  Physical DECtape block boundaries are handled by the controller.

        .text
        .globl start
        .globl __start

__start:
start:
        ; DCT0: device 1 (DTC), device -> processor, move enabled.
        cono 0200,004040

        ; DTC0: selected, start forward, READ DATA.
        cono 0210,0220300

; Discard DAIMON magic; read image_words,,entry_offset into AC2.
header0_wait:
        conso 0200,001000
        jrst header0_wait
        datai 0200,00
header1_wait:
        conso 0200,001000
        jrst header1_wait
        datai 0200,02

; Only the word count is required.  Bound it to the 040000..060000 window.
        hlrz 02,02
        jumpe 02,fail
        caile 02,020000
        jrst fail

        movei 01,040000
read_loop:
        conso 0200,001000
        jrst read_loop
        datai 0200,0(01)
        aoj 01,
        sojg 02,read_loop
        jrst 040000

fail:
        halt .
        jrst fail
