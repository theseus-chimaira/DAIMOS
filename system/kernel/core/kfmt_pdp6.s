/**
 * @file kfmt_pdp6.s
 * @brief Compact resident unsigned 18-bit decimal formatter.
 *
 * This is the PDP-6 baseline implementation.  It intentionally uses only the
 * base instruction set so the same kernel can run on the minimum machine.
 * Later PDP-10 implementations may profit from machine-specific facilities;
 * in particular KL10/KS10-class EXTEND binary/decimal conversion provides a
 * plausible faster formatter path.  Keeping this source explicitly PDP-6
 * leaves room for such optimized variants without weakening the baseline.
 *
 * MonitorFS calls the formatter as a character-at-offset producer instead of
 * formatting into a temporary buffer, keeping resident RAM usage independent
 * of the textual width of the value.
 *
 * Values are limited to one unsigned 18-bit halfword (0..0777777), which is at
 * most six decimal digits.  The six-word power-of-ten table is therefore the
 * complete conversion state; the routine has no writable static storage.
 */
        .text
        .globl  kret_zero
        .globl  kret_one
        .globl  kret_neg1

/** Decimal divisors for the six possible digits of an unsigned 18-bit value. */
kfmt_u18_pow10:
        .long   0303240                 ; 100000 decimal
        .long   023420                  ; 10000 decimal
        .long   01750                   ; 1000 decimal
        .long   0144                    ; 100 decimal
        .long   012                     ; 10 decimal
        .long   1

/**
 * @brief Return one character of an unsigned 18-bit decimal line.
 *
 * C ABI input:
 *   AC1 = unsigned 18-bit value
 *   AC2 = zero-based output character offset
 *   AC3 = address receiving the character
 *
 * C ABI output:
 *   AC1 = 1 when a character was stored, 0 beyond the terminating LF,
 *         or -1 when AC3 is null
 *
 * AC4..AC7 are scratch.  The generated text is the shortest decimal form,
 * followed by CR and LF.  Leading zeroes are suppressed, except that zero is
 * represented as "0".  Negative AC2 values are treated as out of range and
 * return 0 without touching the destination.
 *
 * Digit extraction uses PDP-6 DIV on the AC6/AC7 pair.  For a non-leading
 * digit, the first division discards all higher decimal positions and the
 * second division isolates the requested digit.  This avoids a conversion
 * buffer and keeps the resident implementation small.
 */
        .globl  kfmt_u18_decimal_readchar
kfmt_u18_decimal_readchar:
        jumpe   3,kret_neg1
        movei   4,0
kfmt_u18_first:
        move    6,kfmt_u18_pow10(4)
        caml    1,6
        jrst    kfmt_u18_found
        addi    4,1
        caie    4,5
        jrst    kfmt_u18_first
kfmt_u18_found:
        movei   6,6
        sub     6,4
        jumpl   2,kret_zero
        camge   2,6
        jrst    kfmt_u18_digit
        came    2,6
        jrst    kfmt_u18_lf
        movei   6,015
        jrst    kfmt_u18_store
kfmt_u18_lf:
        addi    6,1
        came    2,6
        jrst    kret_zero
        movei   6,012
        jrst    kfmt_u18_store
kfmt_u18_digit:
        add     4,2
        move    7,1
        setz    6,
        jumpe   4,kfmt_u18_top_digit
        move    5,4
        subi    5,1
        div     6,kfmt_u18_pow10(5)
        setz    6,
        div     6,kfmt_u18_pow10(4)
        jrst    kfmt_u18_digit_ready
kfmt_u18_top_digit:
        div     6,kfmt_u18_pow10(4)
kfmt_u18_digit_ready:
        addi    6,060
kfmt_u18_store:
        movem   6,(3)
        jrst    kret_one
