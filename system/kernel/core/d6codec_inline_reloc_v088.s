/* PDP-6 inline codec v088: relocation-safe, separate from boot low core.
 * Decode semantics and AC state match fixed-address v067, but all internal
 * branches target relocatable symbols and all labels are private to v088.
 * No decoder table, scratch RAM, fixed address or low-core overwrite.
 * Legacy d6lz36_decode_core at 000060 remains active for Stage-1 and EXEC.
 */
        .text
        /* No fixed-address base: each branch uses its relocatable local label. */
        .globl d6codec_inline_decode_reloc_v088
        .globl d6i88_reloc_start
        .globl d6i88_reloc_end
d6codec_inline_decode_reloc_v088:
d6i88_reloc_start:
        ; Validate full destination and current source window before IO.
        ; No literal pools: this decoder is relocatable KCORE text.
        movei   1,1
        lsh     1,022
        move    0,12
        add     0,13
        camle   0,1
        jrst    d6i88_bad
        camge   12,14
        jrst    d6i88_bad
        move    0,3
        add     0,4
        camle   0,1
        jrst    d6i88_bad
        jumpe   13,d6i88_done
        jumpn   11,d6i88_have
d6i88_control:
        sojl    4,d6i88_need
        move    10,0(3)
        aoj     3,
        movei   11,044                 ; 36 tokens
 d6i88_have:
        sojl    4,d6i88_need
        move    0,0(3)
        aoj     3,
        jumpl   10,d6i88_match
        movem   0,0(12)
        aoj     12,
        soja    13,d6i88_next
 d6i88_match:
        move    1,0
        lsh     1,-022                ; reject descriptor high half
        jumpn   1,d6i88_bad
        move    2,0
        lsh     2,-11
        andi    2,0177               ; exact match word count
        jumpe   2,d6i88_bad
        sub     13,2
        jumpl   13,d6i88_bad
        move    1,0
        lsh     1,-020               ; descriptor kind 0/1/2
        andi    0,0777               ; distance in words or halfwords
        jumpe   1,d6i88_copy
        caie    1,1
        jrst    d6i88_half
        jumpn   0,d6i88_bad
 d6i88_zero:
        setzm   0(12)
        aoj     12,
        sojg    2,d6i88_zero
        jrst    d6i88_next
 d6i88_half:
        caie    1,2
        jrst    d6i88_bad
        jumpe   0,d6i88_bad
        trne    0,1
        jrst    d6i88_odd
        lsh     0,-1
        jrst    d6i88_copy
 d6i88_odd:
        aoj     0,
        lsh     0,-1
        move    6,12
        sub     6,0
        camge   6,14
        jrst    d6i88_bad
 d6i88_odd_loop:
        hrrz    7,0(6)
        lsh     7,022
        movem   7,0(12)
        hlrz    7,1(6)
        iorm    7,0(12)
        aoj     6,
        aoj     12,
        sojg    2,d6i88_odd_loop
        jrst    d6i88_next
 d6i88_copy:
        jumpe   0,d6i88_bad
        move    6,12
        sub     6,0
        camge   6,14
        jrst    d6i88_bad
 d6i88_copy_loop:
        move    7,0(6)
        movem   7,0(12)
        aoj     6,
        aoj     12,
        sojg    2,d6i88_copy_loop
 d6i88_next:
        lsh     10,1
        jumpe   13,d6i88_done
        sojg    11,d6i88_have
        jrst    d6i88_control
 d6i88_need:
        movei   0,1
        popj    17,
 d6i88_bad:
        seto    0,
        popj    17,
 d6i88_done:
        setz    0,
        popj    17,
d6i88_reloc_end:
