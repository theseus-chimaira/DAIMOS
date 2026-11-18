; DAIMOS fixed-point decimal, zero digits after the decimal point.
;
; int __daimos_fround0_v33(double value, unsigned long *magnitude,
;                         int *negative);
;
; The double arrives in AC1:AC2 (KCC portable two-word D format),
; magnitude and negative pointers in AC3:AC4.  AC1 returns 0 on success
; and -1 for values with exponents exceeding the 36-bit magnitude range.
;
; Round to nearest, ties to even.  Report the original sign separately
; so -0.25 correctly prints "-0".  No static storage or floating-point
; instruction is needed; the PDP-6 and KA10+ software D-format ABI is
; identical.  Use separate native-FP helpers only if a distinct hardware
; double representation is selected by the compiler.
;
; Registers 10,11,12 are callee-saved in the KCC ABI.  1..7 are scratch.
; All immediate constants and shift counts below are octal.
        .text
        .globl __daimos_fround0_v33
__daimos_fround0_v33:
        push    17,10
        push    17,11
        push    17,12
        setz    7,
        tlne    1,0400000
         movei  7,1
        movem   7,0(4)
        jumpe   7,df0_v33_absdone
        ; Undo the KCC negative two's-complement D-format mantissa.
        setca   1,
        setca   2,
        and     2,[0377777777777]
        addi    2,1
        tlne    2,0400000
         addi   1,1
df0_v33_absdone:
        and     2,[0377777777777]
        move    6,1
        lsh     6,-033
        andi    6,0377
        subi    6,0200
        move    5,1
        and     5,[0777777777]
        ; Below one half, rounding to an integer is always zero.
        jumpl   6,df0_v33_zero
        move    7,6
        subi    7,043
        jumpg   7,df0_v33_error
        ; Choose which word contains the first discarded binary bit.
        move    7,6
        subi    7,033
        jumpg   7,df0_v33_large

        ; 0 <= exponent <= 27.  Whole = hi27 >> (27-exp).
        movei   11,033
        sub     11,6
        move    10,5
        movn    12,11
        lsh     10,0(12)
        jumpe   11,df0_v33_low
        ; Compare the discarded high-word bits with exactly one half.
        movei   7,1
        lsh     7,0(11)
        subi    7,1
        and    7,5
        move    12,11
        subi    12,1
        movei   6,1
        lsh     6,0(12)
        sub     7,6
        jumpg   7,df0_v33_round
        jumpl   7,df0_v33_store
        jumpn   2,df0_v33_round
        trne    10,1
         jrst   df0_v33_round
        jrst    df0_v33_store

df0_v33_low:
        ; 27 exponent bits retained; the first discarded bit is lo35[34].
        move    7,2
        sub     7,[0200000000000]
        jumpg   7,df0_v33_round
        jumpl   7,df0_v33_store
        trne    10,1
         jrst   df0_v33_round
        jrst    df0_v33_store

df0_v33_large:
        ; 28 <= exponent <= 35.  Split the integer across both words.
        move    11,6
        subi    11,033
        move    10,5
        lsh     10,0(11)
        movei   12,043
        sub     12,11
        move    7,2
        movn    6,12
        lsh     7,0(6)
        ior     10,7
        ; Lower (35-k) bits of lo35 are the fractional residue.
        movei   7,1
        lsh     7,0(12)
        subi    7,1
        and     7,2
        move    6,12
        subi    6,1
        movei   11,1
        lsh     11,0(6)
        sub     7,11
        jumpg   7,df0_v33_round
        jumpl   7,df0_v33_store
        trne    10,1
         jrst   df0_v33_round
        jrst    df0_v33_store

df0_v33_round:
        aos     10
df0_v33_store:
        movem   10,0(3)
        setz    1,
        jrst    df0_v33_return
df0_v33_zero:
        setzm   0(3)
        setz    1,
        jrst    df0_v33_return
df0_v33_error:
        seto    1,
df0_v33_return:
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
