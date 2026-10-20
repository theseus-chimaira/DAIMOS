/**
 * @file drm236_mres_v1.s
 * @brief Validated Type-236 block entrypoints installed with drum MRES.
 *
 * The exported KCORE-facing ABI uses a unit, 128-word block and buffer.
 * The Type-167/236 service uses a hardware group address and buffer.
 */
        .text
        .globl drm236_mres_read_v1
        .globl drm236_mres_write_v1
        .globl drm236_read_block_service
        .globl drm236_write_block_service
        .globl kret_neg1

drm236_mres_read_v1:
        setz    4,
        jrst    drm236_mres_block_io_v1

drm236_mres_write_v1:
        movei   4,1

drm236_mres_block_io_v1:
        jumpe   3,kret_neg1
        trne    1,0777774              ; unit 0..3
        jrst    kret_neg1
        tlne    2,0777777              ; block fits RH18
        jrst    kret_neg1
        cail    2,020000               ; 8192 blocks per drum
        jrst    kret_neg1
        ; Type-236 group = (unit << 16) | (block << 3)
        lsh     1,020
        lsh     2,3
        ior     1,2
        move    2,3
        jumpe   4,drm236_read_block_service
        jrst    drm236_write_block_service
