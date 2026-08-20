; PDP-6 HDD/DBOOT V1 Stage1.
;
; This is the paper-tape/RIM Stage1 image.
; It scans DSK270 units 0..3, accepts
; compact DBC or DB0/DB1/DBX metadata,
; reconstructs a round-robin opaque
; image stream across one to four
; members, skips each member's bad-run
; table, loads the image at 040000,
; and jumps to its relative entry point.

        .text
        .include "../common/sixbit-output.inc"

        .globl __start
        .entry __start
__start:
        movei 017,073040
        setom 000040
        setom 000041
        setzm any_read_ok
        setzm found_mask
        setzm member_mask
        setom current_unit
stage1_unit_loop:
        aos 02,current_unit
        cail 02,04
        jrst stage1_units_done
stage1_try_unit:
        pushj 017,locate_unit
        jumpe 01,stage1_next_unit
        move 02,03
        movei 03,01
        lsh 03,0(02)
        iorb 03,found_mask
        camn 03,member_mask
        jrst stage1_units_done
stage1_next_unit:
        jrst stage1_unit_loop

stage1_units_done:
        move 02,any_read_ok
        jumpe 02,fail_nodsk
        move 05,member_mask
        jumpe 05,fail_noset
        andcm 05,found_mask
        jumpn 05,fail_noset

stage1_have_set:
        setom stream_member
        pushj 017,read_next_stream_sector
        jumpe 01,fail_khead

        move 02,buffer
        came 02,daimon_magic
        jrst fail_khead
stage1_magic_ok:
        hlrz 03,buffer+000001
        hrrz 04,buffer+000001
        movei 05,040000
        add 05,04
        movem 05,entry_addr

        movei 010,040000
        move 011,03
        movei 06,buffer+000002
        movei 07,0176
        pushj 017,copy_stream_words
        jumpe 01,load_image_done
load_image_loop:
        pushj 017,read_next_stream_sector
        jumpe 01,fail_read
        movei 06,buffer
        movei 07,0200
        pushj 017,copy_stream_words
        jumpn 01,load_image_loop
load_image_done:
        movei 017,050000
        setz 01,
        move 02,member_count
        jrst @entry_addr

; Locate DBOOT on current_unit.
; AC1 = 1 if a usable descriptor was stored.
locate_unit:
        setzm bad_count
        setom scan_sector
locate_scan_next:
        aos 02,scan_sector
        cail 02,0200
        jrst return_zero
        pushj 017,unit_sector_to_dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        aos any_read_ok
        hlrz 02,buffer
        cain 02,0444243
        jrst locate_dbc
        caie 02,0444220
        jrst return_zero

locate_db0:
        move 02,scan_sector
        movem 02,boot_locator
        pushj 017,parse_db0_badmap
        jumpe 01,locate_scan_next
        skipn db1_count
        jrst locate_no_db1
        hrrz 02,buffer+000021
        movem 02,last_badmap_sector
        pushj 017,unit_sector_to_dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        move 06,db1_count
        addm 06,bad_count
        movei 07,bad_words+000020
        hrli 07,buffer
        blt 07,bad_words+000217
locate_no_db1:
        aos 02,last_badmap_sector
        movei 05,bad_words
        move 04,bad_count
        pushj 017,bad_map_skip
        movem 02,located_dboot_loc
        pushj 017,unit_sector_to_dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        hlrz 02,buffer
        caie 02,0444270
        jrst locate_scan_next

parse_descriptor:
        move 02,buffer+000005
        move 03,02
        lsh 03,-020
        andi 03,017
        cail 03,04
        jrst return_zero
parse_desc_index_ok:
        move 04,02
        lsh 04,-024
        move 05,02
        lsh 05,-014
        andi 05,017
        jumpe 05,return_zero
parse_desc_member_ok:
        movei 07,01
        lsh 07,0(03)
        tdnn 07,04
        jrst return_zero

        move 01,member_mask
        jumpe 01,parse_desc_first
        came 01,04
        jrst return_zero
parse_desc_mask_ok:
        move 01,member_count
        came 01,05
        jrst return_zero
        jrst parse_desc_store

parse_desc_first:
        movem 04,member_mask
        movem 05,member_count

parse_desc_store:
        move 05,current_unit
        movem 05,member_unit(03)
        lsh 05,020
        ior 05,boot_locator
        move 06,03
        lsh 06,-01
        addi 06,000040
        trne 03,01
        jrst parse_desc_handoff_rh
        hrlm 05,0(06)
        jrst parse_desc_handoff_done
parse_desc_handoff_rh:
        hrrm 05,0(06)
parse_desc_handoff_done:
        move 05,located_dboot_loc
        aoj 05,
        movem 05,member_next_sector(03)
        move 05,bad_count
        movem 05,member_bad_count(03)
        move 07,03
        lsh 07,04
        move 06,03
        lsh 06,07
        add 07,06
        addi 07,member_bad_words
        movem 07,member_bad_ptr(03)
        movei 06,000217(07)
        hrli 07,bad_words
        blt 07,0(06)
        movei 01,01
        popj 017,
; DB0 RH: VERSION3 | DB0_RUN_COUNT6 | DB1_RUN_COUNT9.
; Bad runs are packed two 18-bit descriptors per word.
parse_db0_badmap:
        hrrz 03,buffer
        move 04,03
        andi 04,0777
        movem 04,db1_count
        move 06,03
        lsh 06,-011
        andi 06,077
        movem 06,bad_count
        movei 07,bad_words
        hrli 07,buffer+000001
        blt 07,bad_words+000017
        move 02,scan_sector
        movem 02,last_badmap_sector
        jrst return_one

; Advance AC2 past every bad run containing it.
; AC4 = run count, AC5 = packed bad-run table.
bad_map_skip:
        jumpe 04,bad_map_skip_done
bad_map_word_loop:
        hlrz 03,0(05)
        pushj 017,bad_half_skip
        soje 04,bad_map_skip_done
        hrrz 03,0(05)
        pushj 017,bad_half_skip
        aoj 05,
        sojg 04,bad_map_word_loop
bad_map_skip_done:
        popj 017,

; AC3 = START_SECTOR10 | RUN_LENGTH_MINUS_ONE8, AC2 = candidate.
bad_half_skip:
        move 07,03
        lsh 07,-010
        camge 02,07
        popj 017,
        andi 03,0377
        addi 03,01
        add 03,07
        caml 02,03
        popj 017,
        move 02,03
        popj 017,

locate_descriptor:
        movem 02,located_dboot_loc
        jrst parse_descriptor

locate_dbc:
        movei 02,000377
        movem 02,boot_locator
        skipn 02,scan_sector
        jrst locate_descriptor
return_zero:
        setz 01,
        popj 017,

read_next_stream_sector:
read_next_member:
        aos 06,stream_member
        andi 06,03
        movei 03,01
        lsh 03,0(06)
        tdnn 03,member_mask
        jrst read_next_member
        movem 06,stream_member
        move 02,member_next_sector(06)
        pushj 017,skip_member_bad_sectors
        movem 02,member_next_sector(06)
        aos member_next_sector(06)
        move 05,member_unit(06)
        pushj 017,linear_to_dsk_addr
        lsh 05,020
        add 02,05
        jrst read_dsk_sector

skip_member_bad_sectors:
        move 05,member_bad_ptr(06)
        move 04,member_bad_count(06)
        jrst bad_map_skip

copy_stream_words:
        jumpe 011,copy_stream_done
copy_stream_loop:
        move 04,0(06)
        movem 04,0(010)
        aoj 06,
        aoj 010,
        soje 011,copy_stream_done
        sojg 07,copy_stream_loop
        jrst return_one
copy_stream_done:
        jrst return_zero

unit_sector_to_dsk_addr:
        pushj 017,linear_to_dsk_addr
        move 05,current_unit
        lsh 05,020
        add 02,05
        popj 017,

linear_to_dsk_addr:
        idivi 02,000054
        lsh 02,06
        add 02,03
        popj 017,

read_dsk_sector:
        cono 0200,04000
        datao 0270,02
        pushj 017,wait_dfr
        jumpe 01,read_dsk_fail
        cono 0270,01000
        movsi 02,-0200
read_loop:
        pushj 017,wait_dct_rq
        datai 0200,03
        movem 03,buffer(02)
        aobjn 02,read_loop
        cono 0270,030000
        pushj 017,wait_ids
        jumpe 01,read_dsk_fail
        jrst return_one
read_dsk_fail_end:
read_dsk_fail:
        cono 0270,030000
        jrst return_zero

wait_dfr:
        movei 03,040000
        jrst wait_dsk_status

wait_ids:
        movei 03,0400000

wait_dsk_status:
        coni 0270,05
        trne 05,01777
        jrst return_zero
        and 05,03
        jumpn 05,return_one
        jrst wait_dsk_status

wait_dct_rq:
        coni 0200,05
        trne 05,01000
        jrst return_one
        jrst wait_dct_rq
return_one:
        movei 01,01
        popj 017,

fail_nodsk:
        movei 01,000001
        jrst fail_common
fail_noset:
        movei 01,000002
        jrst fail_common
fail_khead:
        movei 01,000003
        jrst fail_common
fail_read:
        movei 01,000004
fail_common:
        move 05,01
        move 01,msg_nodsk-1(05)
        pushj 017,put_sixbit_word
        move 01,05
halt_stage1:
        movem 01,stage1_last_error
        halt .
        jrst halt_stage1


daimon_magic: .word 0444151555756
msg_nodsk:    .word 0375657446353
msg_noset:    .word 0375657634564
msg_khead:    .word 0375350454144
msg_read:     .word 0376245414400
        .bss
any_read_ok: .block 01
current_unit: .block 01
scan_sector: .block 01
found_mask: .block 01
member_mask: .block 01
member_count: .block 01
located_dboot_loc: .block 01
boot_locator: .block 01
bad_count: .block 01
db1_count: .block 01
last_badmap_sector: .block 01
stream_member: .block 01
entry_addr: .block 01
stage1_last_error: .block 01
member_unit: .block 04
member_bad_count: .block 04
member_next_sector: .block 04
member_bad_ptr: .block 04
bad_words: .block 0220
member_bad_words: .block 01100
buffer: .block 0200
stage1_bss_end:
