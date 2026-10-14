/* KCORE relocatable inline decoder. No low-core alias. */
/* Permanent KCORE namespaced inline decoder; legacy core remains separate. */
/* Experimental inline-token D6 codec v067, fixed-address PDP-6 decoder.
 * One 36-bit control word per <=36 interleaved literal/descriptor tokens.
 * Unlike paired-format v058, descriptor words remain interleaved with
 * literals. Uses the legacy streaming AC3/AC4/AC10..AC14/AC0 ABI.
 * No static data, stack, or new VFS refill buffering. Not production-linked.
 */
        .text
        .globl d6codec_inline_core_v088
        .globl d6codec_inline_decode_v088
        .globl d6i67_image_start
        .globl d6i67_image_end
d6codec_inline_decode_v088:
d6codec_inline_core_v088:
d6i67_image_start:
        ; Validate full destination and current source window before IO.
        ; No literal pools: this image is copied to fixed low core.
        movei   1,1
        lsh     1,022
        move    0,12
        add     0,13
        camle   0,1
        jrst    d6i67_bad
        camge   12,14
        jrst    d6i67_bad
        move    0,3
        add     0,4
        camle   0,1
        jrst    d6i67_bad
        jumpe   13,d6i67_done
        jumpn   11,d6i67_have
d6i67_control:
        sojl    4,d6i67_need
        move    10,0(3)
        aoj     3,
        movei   11,044                 ; 36 tokens
 d6i67_have:
        sojl    4,d6i67_need
        move    0,0(3)
        aoj     3,
        jumpl   10,d6i67_match
        movem   0,0(12)
        aoj     12,
        soja    13,d6i67_next
 d6i67_match:
        move    1,0
        lsh     1,-022                ; reject descriptor high half
        jumpn   1,d6i67_bad
        move    2,0
        lsh     2,-11
        andi    2,0177               ; exact match word count
        jumpe   2,d6i67_bad
        sub     13,2
        jumpl   13,d6i67_bad
        move    1,0
        lsh     1,-020               ; descriptor kind 0/1/2
        andi    0,0777               ; distance in words or halfwords
        jumpe   1,d6i67_copy
        caie    1,1
        jrst    d6i67_half
        jumpn   0,d6i67_bad
 d6i67_zero:
        setzm   0(12)
        aoj     12,
        sojg    2,d6i67_zero
        jrst    d6i67_next
 d6i67_half:
        caie    1,2
        jrst    d6i67_bad
        jumpe   0,d6i67_bad
        trne    0,1
        jrst    d6i67_odd
        lsh     0,-1
        jrst    d6i67_copy
 d6i67_odd:
        aoj     0,
        lsh     0,-1
        move    6,12
        sub     6,0
        camge   6,14
        jrst    d6i67_bad
 d6i67_odd_loop:
        hrrz    7,0(6)
        lsh     7,022
        movem   7,0(12)
        hlrz    7,1(6)
        iorm    7,0(12)
        aoj     6,
        aoj     12,
        sojg    2,d6i67_odd_loop
        jrst    d6i67_next
 d6i67_copy:
        jumpe   0,d6i67_bad
        move    6,12
        sub     6,0
        camge   6,14
        jrst    d6i67_bad
 d6i67_copy_loop:
        move    7,0(6)
        movem   7,0(12)
        aoj     6,
        aoj     12,
        sojg    2,d6i67_copy_loop
 d6i67_next:
        lsh     10,1
        jumpe   13,d6i67_done
        sojg    11,d6i67_have
        jrst    d6i67_control
 d6i67_need:
        movei   0,1
        popj    17,
 d6i67_bad:
        seto    0,
        popj    17,
 d6i67_done:
        setz    0,
        popj    17,
d6i67_image_end:
