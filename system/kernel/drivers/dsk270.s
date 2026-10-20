/**
 * @file dsk270.s
 * @brief Two-word KCORE dispatch for optional Type-270 disk hardware.
 *
 * MINIT patches these jump words to the public, validated MRES entrypoints.
 * Without installed disk hardware they return a storage error.  All Type-270
 * address conversion and validation live in dsk270_mres_v1.s.
 */
        .text
        .globl dsk270_read_sector
        .globl dsk270_write_sector
        .globl dsk270_read_jump
        .globl dsk270_write_jump
        .globl kret_neg1

dsk270_read_sector:
dsk270_read_jump:
        jrst kret_neg1

dsk270_write_sector:
dsk270_write_jump:
        jrst kret_neg1
