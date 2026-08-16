; Common Stage1 body for sequential opaque-image boot devices.
;
; Backend contract:
;   read_words: AC1 = destination, AC2 = word count.
;               Return after storing exactly AC2 words.
;
; The including backend initializes its device before entering this body.

        movei 01,header
        movei 02,02
        pushj 017,read_words
        move 02,header
        camn 02,daimon_magic
        jrst header_ok
        jrst fail
header_ok:
        hlrz 02,header+01
        jumpe 02,fail
        hrrz 03,header+01
        caml 03,02
        jrst fail
        movei 04,040000
        add 04,02
        caile 04,060000
        jrst fail
        movei 04,040000
        add 04,03
        movem 04,entry_addr
        movei 01,040000
        pushj 017,read_words
        movei 017,050000
        setz 01,
        setz 02,
        jrst @entry_addr
