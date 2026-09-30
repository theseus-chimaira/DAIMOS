/**
 * @file drm236.s
 * @brief Generic KCORE bridge to the PDP-6 Type 167/236 resident driver.
 *
 * The public interface transfers one 128-word physical block. Type 236 media
 * addresses use 16-word groups, so a DAIMOS block number is shifted left by
 * three. Address bits 16..17 select one of four drum units. MINIT patches the
 * two tail jumps below to the relocated MRES services; when DRM is absent they
 * retain their safe failure targets supplied by the normal module setup.
 *
 * No PDP-10-only instruction is used, so this bridge has no model suffix.
 */
        .text
        .globl  drm236_read_block
        .globl  drm236_write_block
        .globl  drm236_read_jump
        .globl  drm236_write_jump
        .globl  kret_neg1

/**
 * @brief Read one validated Type 236 block through the patched MRES tail.
 * @param AC1 Drum unit 0..3.
 * @param AC2 Physical block 0..017777.
 * @param AC3 Destination buffer address, nonzero.
 * @return AC1 = 0 on success or -1 on validation/I/O failure.
 *
 * AC2 is clobbered while forming the hardware group address. AC4 selects the
 * read/write tail. AC17 remains the caller's normal stack.
 */
drm236_read_block:
        setz    4,
        jrst    drm236_block_io

drm236_write_block:
        movei   4,1

drm236_block_io:
        jumpe   3,kret_neg1
        trne    1,0777774              ; units 0..3 only
        jrst    kret_neg1
        tlne    2,0777777              ; block must fit RH18
        jrst    kret_neg1
        cail    2,020000               ; 8192 blocks per drum
        jrst    kret_neg1

        ; Type-236 address = unit<<16 | block<<3 (16-word groups).
        lsh     1,020
        lsh     2,3
        ior     1,2
        move    2,3                    ; MRES service AC2 = buffer
        jumpe   4,drm236_read_jump

drm236_write_jump:
        jrst    0                       ; patched by MINIT

drm236_read_jump:
        jrst    0                       ; patched by MINIT
