/**
 * @file dsk270.s
 * @brief Generic KCORE bridge to the resident PDP-6 Type 270 disk service.
 *
 * The public API remains unit/sector based. Validation and conversion to the
 * Type 270 raw address live here so the optional MRES does not carry a C helper
 * frame on every operation. MINIT patches the read/write tail jumps to the
 * relocated DSK services when the hardware is present.
 *
 * No PDP-10-only instruction is used, so this bridge has no model suffix.
 */
        .text
        .globl  dsk270_read_sector
        .globl  dsk270_write_sector
        .globl  dsk270_read_jump
        .globl  dsk270_write_jump
        .globl  kret_neg1

/**
 * @brief Read one validated physical Type 270 sector.
 * @param AC1 Unit 0..3.
 * @param AC2 Sector 0..0127777 (45055 decimal).
 * @param AC3 Nonzero 128-word destination buffer.
 * @return AC1 = 0 on success or negative storage error on failure.
 *
 * AC5/AC6 are clobbered by IDIVI while converting linear sector to cylinder
 * and sector-within-cylinder. AC4 selects the patched read/write tail.
 */
dsk270_read_sector:
        setz    4,                      ; read selector
        jrst    dsk270_sector_io

dsk270_write_sector:
        movei   4,1                    ; write selector

dsk270_sector_io:
        jumpe   3,kret_neg1
        trne    1,0777774              ; units 0..3 only
        jrst    kret_neg1
        tlne    2,0777777              ; sector must fit RH18
        jrst    kret_neg1
        cail    2,0130000              ; 02000 cyl * 054 sectors/cyl
        jrst    kret_neg1

dsk270_sector_addr:
        ; raw = unit<<16 | (sector/054)<<6 | sector%054
        move    5,2
        idivi   5,054                   ; AC5 quotient, AC6 remainder
        lsh     1,020
        lsh     5,6
        ior     1,5
        ior     1,6
        move    2,3
        jumpe   4,dsk270_read_jump

dsk270_write_jump:
        jrst    0                       ; patched by MINIT

dsk270_read_jump:
        jrst    0                       ; patched by MINIT
