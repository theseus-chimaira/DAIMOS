; stage1.s -- minimal opaque-image Stage1 for PDP-6 magnetic tape.
;
; Stage0 loads this loader from RIM paper tape.  Stage1 reads one magnetic-tape
; record through a Type 516 control and Type 136 data control.
;
; The record contains:
;       words 0-15  fixed low-core SIXBIT helper -> 000060..000077
;       remainder   completely opaque boot image -> 040000...
;
; The magnetic-tape record boundary terminates the image.  There is no DAIMON
; header, word count, entry offset, checksum, or image validation.  Once the
; record has been drained, execution starts at 040000.
;
; After the low-core helper has been installed, Stage1 reports medium/controller
; read failures as "?RDERR" and tape mark/end-of-medium as "?BADTP".
;
; Do not test the Type 516 PARITY_ERR bit here.  The historical PDP-6 SIMH
; 7-track implementation asserts it spuriously on ordinary legacy tape images.
; PARITY_ERRL and MIS_CHR remain reliable error indications.

        .text
        .globl start
        .globl __start

__start:
start:
        ; Type 136: input, six 6-bit characters, device 3, move enabled.
        movei 01,004000
        cono 0200,0(01)

        ; Type 516: unit 0, 556 bpi, binary parity, read forward.
        movei 01,052400
        cono 0220,0(01)

        ; The first 16 words install the fixed helper at 000060.  Thereafter
        ; the same transfer loop writes the opaque image at 040000.
        movei 01,000060

read_loop:
        conso 0200,001000
        jrst read_wait
        datai 0200,0(01)
        aoj 01,
        caie 01,000100
        jrst read_loop
        movei 01,040000
        jrst read_loop

read_wait:
        ; Before 000100 the diagnostic helper is not complete, so malformed
        ; or short records can only be halted safely.
        caige 01,040000
        jrst lowcore_wait

        ; PARITY_ERR is intentionally omitted; see file comment above.
        consz 0224,0000120
        jrst read_error
        consz 0224,0400400
        jrst bad_tape
        conso 0224,0000004
        jrst read_loop
        ; EOR may precede delivery of the final DCT word.
        consz 0200,0002000
        jrst read_loop

        ; At least one opaque image word must have been transferred.
        caie 01,040000
        jrst 040000
        jrst bad_tape

lowcore_wait:
        ; ILL_OPR, EOF_FLAG, PARITY_ERRL, or MIS_CHR before the helper is
        ; complete cannot be diagnosed because 000060 is not yet usable.
        consz 0224,0400520
        halt .
        conso 0224,0000004
        jrst read_loop
        ; If EOR is set, allow a final word already pending in the DCT.
        conso 0200,0003000
        halt .
        jrst read_loop

read_error:
        move 01,msg_rderr
        jrst diag

bad_tape:
        move 01,msg_badtp

diag:
        movei 017,050000
        pushj 017,000060
        halt .
        jrst diag

msg_rderr: .word 0376244456262
msg_badtp: .word 0374241446460
