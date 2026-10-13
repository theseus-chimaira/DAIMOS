/* D6 production codec ABI adapter, staged pending atomic format cutover.
 * C arguments: AC1 source; AC2 input words; AC3 destination; AC4 limit.
 * Encoder returns AC1 encoded words/-1; decoder returns AC1 zero/-1.
 * The native v053 engines clobber AC10..AC16, so preserve them here.
 * AC17 is the process-private kernel pushdown pointer.
 */
        .text
        .globl d6codec_kcore_encode_v055
        .globl d6codec_kcore_decode_v055
        .globl d6codec_v053_encode_literals
        .globl d6codec_v053_decode

d6codec_kcore_encode_v055:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        pushj   17,d6codec_v053_encode_literals
        jrst    d6codec_kcore_restore_v055

d6codec_kcore_decode_v055:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        pushj   17,d6codec_v053_decode

d6codec_kcore_restore_v055:
        pop     17,16
        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
