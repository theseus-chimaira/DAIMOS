; Common Stage1 body for sequential opaque-image boot devices.
;
; Backend contract:
;   read_words: AC1 = destination, AC2 = nonzero word count.
;               Return after storing exactly AC2 words.
;
; The including backend initializes its device before entering this body.

        movei 01,header
        movei 02,02
        pushj 017,read_words
        move 02,header
        move 06,header+01
        .include "../common/validate-load.inc"
        movei 01,040000
        pushj 017,read_words
        movei 017,050000
        setzb 01,02
        jrst 0(07)
