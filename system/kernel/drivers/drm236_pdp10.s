; drm236_pdp10.s -- compact KCORE bridge to the PDP-6 Type 167/236 MRES.
;
; The public interface is one 128-word Type-236 physical block.  The drum
; control addresses media in 16-word groups, so each DAIMOS block advances
; the low 16-bit group address by eight.  Bits 16-17 select one of four drums.
        .text
        .globl  drm236_read_block
        .globl  drm236_write_block
        .globl  drm236_read_jump
        .globl  drm236_write_jump
        .globl  pdp10_ret_neg1

; int drm236_read_block(unsigned unit, kword_t block, kword_t *buf)
; int drm236_write_block(unsigned unit, kword_t block, const kword_t *buf)
; C arguments: AC1 unit, AC2 block, AC3 buffer.
drm236_read_block:
        setz    4,
        jrst    drm236_block_io

drm236_write_block:
        movei   4,1

drm236_block_io:
        jumpe   3,pdp10_ret_neg1
        trne    1,0777774              ; units 0..3 only
        jrst    pdp10_ret_neg1
        tlne    2,0777777              ; block must fit RH18
        jrst    pdp10_ret_neg1
        cail    2,020000               ; 8192 blocks per drum
        jrst    pdp10_ret_neg1

        ; Type-236 address = unit<<16 | block<<3 (16-word groups).
        lsh     1,020
        move    5,2
        lsh     5,3
        ior     1,5
        move    2,3                    ; MRES service AC2 = buffer
        jumpe   4,drm236_read_jump

drm236_write_jump:
        jrst    0                       ; patched by MINIT

drm236_read_jump:
        jrst    0                       ; patched by MINIT
