/**
 * @file dsk270_mres_v1.s
 * @brief Type-270 sector validation and address conversion in disk MRES.
 *
 * This code is installed only when MINIT detects a Type-270 controller.
 * Public calls retain the unit/sector/buffer ABI.  The existing raw MRES
 * service takes a converted Type-270 address and the buffer in AC2.
 */
        .text
        .globl dsk270_mres_read_v1
        .globl dsk270_mres_write_v1
        .globl dsk_read_sector
        .globl dsk_write_sector
        .globl kret_neg1

dsk270_mres_read_v1:
        setz    4,
        jrst    dsk270_mres_sector_io_v1

dsk270_mres_write_v1:
        movei   4,1

dsk270_mres_sector_io_v1:
        jumpe   3,kret_neg1
        trne    1,0777774              ; unit 0..3
        jrst    kret_neg1
        tlne    2,0777777              ; sector fits RH18
        jrst    kret_neg1
        cail    2,0130000              ; 02000 cylinders * 054 sectors
        jrst    kret_neg1
        ; raw = (unit << 16) | ((sector / 054) << 6) | sector % 054
        move    5,2
        idivi   5,054
        lsh     1,020
        lsh     5,6
        ior     1,5
        ior     1,6
        move    2,3
        jumpe   4,dsk_read_sector
        jrst    dsk_write_sector
