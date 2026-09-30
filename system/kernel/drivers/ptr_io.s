/**
 * @file ptr_io.s
 * @brief Resident PDP-6 paper-tape reader driver for device 0104.
 *
 * PTR owns a local PI7 leaf so a PTR-only machine does not load unrelated PI7
 * peripherals. ptr_state has three meanings: zero is idle/no byte, -1 is an
 * outstanding caller-started read, and a positive value is byte+1 prefetched
 * by PI service. The +1 representation keeps NUL distinct from empty state.
 *
 * When DONE arrives without an active caller, the handler leaves the hardware
 * byte pending and disables PI. A later caller consumes it directly with
 * DATAI. This provides one-byte hardware-assisted prefetch without extra BSS.
 */
        .globl mfsdev_io_in
        .text
        .globl ptr_pi_handler
        .globl ptr_getchar
        .globl ptr_read_words
        .globl pdp10_pi_handler_return
        .globl kret_arg
        .globl kret_busy
        .globl kret_ok

/**
 * @brief Service PTR DONE on PI7.
 * @return Does not return normally; jumps to pdp10_pi_handler_return.
 *
 * AC1 is untouched and AC2/AC3/AC17 are preserved for the generic PI ABI. If
 * ptr_state is -1, DATAI stores the byte and AOS converts it to byte+1 before
 * PI is disabled. If no caller owns the request, DONE is retained as a
 * hardware-prefetched byte with PI disabled.
 */
ptr_pi_handler:
        conso 0104,0010
        jrst pdp10_pi_handler_return
        skipn ptr_state
        jrst ptr_pi_prefetch
        datai 0104,ptr_state
        aos mfsdev_io_in+2
        aos ptr_state
        cono 0104,0
        jrst pdp10_pi_handler_return
ptr_pi_prefetch:
        cono 0104,0010
        jrst pdp10_pi_handler_return

/**
 * @brief Read one eight-bit PTR byte into caller storage.
 * @param AC1 Address of destination integer; must be nonzero.
 * @return AC1 = 0, PT_E_ARG (-1), PT_E_TIMEOUT (-2), or PT_E_BUSY (-3).
 *
 * AC4 preserves the destination pointer; AC2/AC3/AC5 are scratch. If DONE is
 * already set the byte is consumed synchronously. Otherwise ptr_state becomes
 * -1 before CONO starts the reader, closing the completion race. Timeout
 * disables PI and releases software ownership; a delayed hardware completion
 * remains available for the next call.
 */
ptr_getchar:
        jumpe 1,kret_arg
        move 4,1
        move 2,ptr_state
        jumpg 2,ptr_get_software
        jumpl 2,kret_busy
        consz 0104,0010
        jrst ptr_get_hardware
        setom ptr_state
        cono 0104,0027
        movei 5,0200000
ptr_get_wait:
        move 2,ptr_state
        jumpg 2,ptr_get_software
        sojg 5,ptr_get_wait
        setzm ptr_state
        cono 0104,0
        jrst kret_neg2
ptr_get_hardware:
        datai 0104,3
        aos mfsdev_io_in+2
        cono 0104,0
        andi 3,0377
        movem 3,(4)
        jrst ptr_get_ok
ptr_get_software:
        subi 2,1
        andi 2,0377
        movem 2,(4)
        setzm ptr_state
ptr_get_ok:
        jrst kret_ok

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
        skipe ptr_state
        jrst kret_busy
        push 17,010
        push 17,011
        push 17,012
        push 17,013
        push 17,014
        push 17,015
        move 010,1                    ; output cursor
        hrrz 011,2                    ; requested output words
        setz 012,                     ; completed output words
        setom ptr_state               ; exclude character/other bulk readers
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
        setzm ptr_state
        move 1,012
        pop 17,015
        pop 17,014
        pop 17,013
        pop 17,012
        pop 17,011
        pop 17,010
        popj 17,

        .bss
/** 0 idle, -1 active request, positive byte+1 completed/prefetched value. */
ptr_state:
        .block 1
