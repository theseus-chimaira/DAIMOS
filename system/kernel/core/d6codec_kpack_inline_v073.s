/* Staged PDP-6 KPACK codec adapters for the inline wire format v073.
 * Encoder: AC1 source, AC2 words, AC3 destination, AC4 capacity.
 * Decoder: AC1 source, AC2 compressed words, AC3 destination,
 *          AC4 expected output words.  Return AC1 decoded count or -1.
 * Preserve AC10..AC16 across calls. No global scratch or allocations.
 */
.text
.globl d6codec_kpack_enc_v073
.globl d6codec_kpack_dec_v073
.globl d6codec_inline_encode_v071
.globl d6inline_decode_buffer_v070

d6codec_kpack_enc_v073:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        pushj   17,d6codec_inline_encode_v071
        jrst    d6ki73_restore

d6codec_kpack_dec_v073:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        ; Construct exact output count,,output pointer in AC4.
        ; Reject unrepresentable count and pointer before truncation.
        move    5,4
        lsh     5,-022
        jumpn   5,d6ki73_bad
        move    5,3
        lsh     5,-022
        jumpn   5,d6ki73_bad
        move    5,4
        lsh     5,022
        ior     5,3
        move    3,2
        move    2,1
        move    4,5
        push    17,4
        pushj   17,d6inline_decode_buffer_v070
        pop     17,5
        jumpn   1,d6ki73_bad
        hlrz    1,5
        jrst    d6ki73_restore
 d6ki73_bad:
        seto    1,
 d6ki73_restore:
        pop     17,16
        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
