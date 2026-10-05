/**
 * @file ptr_io.s
 * @brief Resident PDP-6 paper-tape reader driver for device 0104.
 *
 * Runtime input is the native WORDTOKEN8 bulk path.  KINIT probes the reader
 * with PI assignment enabled, then disables the PIA before publishing this
 * MRES.  Runtime transfer is synchronous and never sleeps, so the executive
 * cannot switch to another process while the physical reader is owned.
 */
        .globl mfsdev_io_in
        .text
        .globl ptr_read_words
        .globl kret_arg
        .globl kret_busy
        .globl kret_ok

; int ptr_read_words(kword_t *words, unsigned int nwords)
; Pack four physical bytes into bits 35..4.  Bulk input owns the PTR for the
; whole call and runs it continuously, matching the Stage1 transport: one
; CONO starts motion and successive DONE/DATAI cycles consume bytes.  This is
; both faster and correct for the Type 760; restarting the reader for every
; byte can advance past alternate bytes.  A full word leaves the low nibble
; zero; timeout after 1..3 bytes emits a partial word with its byte count in
; the low nibble, while timeout before a byte is stream EOF.
ptr_read_words:
        jumpe 1,kret_arg
        jumpe 2,kret_ok
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        push 17,015
        move 010,1                    ; output cursor
        hrrz 011,2                    ; requested output words
        setz 012,                     ; completed output words
        cono 0104,0020                ; continuous reader, PI disabled
ptr_words_next:
        setz 013,                     ; packed byte accumulator
        setz 014,                     ; bytes in current word
ptr_words_byte:
        movei 015,0200000             ; bounded mechanical wait
ptr_words_wait:
        consz 0104,0010               ; DONE
        jrst ptr_words_have_byte
        sojg 015,ptr_words_wait
        jrst ptr_words_stop
ptr_words_have_byte:
        datai 0104,015
        aos mfsdev_io_in+2
        lsh 013,010                   ; append one byte
        andi 015,0377
        ior 013,015
        addi 014,1
        caige 014,4
        jrst ptr_words_byte
        lsh 013,4                     ; canonical WORDTOKEN8: low nibble zero
        movem 013,(010)
        addi 010,1
        addi 012,1
        sojg 011,ptr_words_next
        jrst ptr_words_return
ptr_words_stop:
        jumpe 014,ptr_words_no_partial
        movei 015,044                 ; 36 - 8*valid bytes
        move 2,014
        imuli 2,010
        sub 015,2
        lsh 013,0(015)
        ior 013,014                   ; partial byte count in low nibble
        movem 013,(010)
        addi 012,1
        jrst ptr_words_return
ptr_words_no_partial:
        ; No hardware error status exists on this interface.  A bounded wait
        ; with no next byte is therefore the physical end-of-stream condition.
ptr_words_return:
        cono 0104,0
        move 1,012
        pop 17,015
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,
