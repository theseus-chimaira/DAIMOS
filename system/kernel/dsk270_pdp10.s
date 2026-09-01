; dsk270_pdp10.s -- compact KCORE bridge to the resident DSK270 MRES service.
;
; The public C-facing API remains sector based.  Unit/sector validation and
; Type-270 address formation live here so the physical driver does not spend a
; generated-C helper frame on every operation.
        .text
        .globl  dsk270_read_sector
        .globl  dsk270_write_sector
        .globl  dsk270_read_addr_v1
        .globl  dsk270_write_addr_v1

; int dsk270_read_sector(unsigned unit, kword_t sector, kword_t *buf)
; int dsk270_write_sector(unsigned unit, kword_t sector, const kword_t *buf)
; C arguments: AC1 unit, AC2 sector, AC3 buffer.
dsk270_read_sector:
        move    4,dsk270_read_addr_v1
        jrst    dsk270_sector_io

dsk270_write_sector:
        move    4,dsk270_write_addr_v1

; AC4 is the relocated MRES service address.
dsk270_sector_io:
        jumpe   4,dsk270_sector_bad
        jumpe   3,dsk270_sector_bad
        trne    1,0777774              ; units 0..3 only
        jrst    dsk270_sector_bad
        tlne    2,0777777              ; sector must fit RH18
        jrst    dsk270_sector_bad
        caige   2,0130000              ; 02000 cyl * 054 sectors/cyl
        jrst    dsk270_sector_addr

dsk270_sector_bad:
        seto    1,
        popj    17,

dsk270_sector_addr:
        ; raw = unit<<16 | (sector/054)<<6 | sector%054
        setz    5,
        move    6,2
        divi    5,054                   ; AC5 quotient, AC6 remainder
        lsh     1,020
        lsh     5,6
        ior     1,5
        ior     1,6
        move    2,3
        andi    4,0777777
        pushj   17,(4)
        popj    17,

        .bss
dsk270_read_addr_v1:  .block 1
dsk270_write_addr_v1: .block 1
