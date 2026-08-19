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
        setzm any_read_ok
        setzm found_mask
        setzm member_mask
        setzm current_unit
stage1_unit_loop:
        move 02,current_unit
        cail 02,04
        jrst stage1_units_done
stage1_try_unit:
        pushj 017,locate_unit
        jumpe 01,stage1_next_unit
        move 02,03
        movei 03,01
        lsh 03,0(02)
        iorb 03,found_mask
        and 03,member_mask
        camn 03,member_mask
        jrst stage1_units_done
stage1_next_unit:
        aos current_unit
        jrst stage1_unit_loop

stage1_units_done:
        move 02,any_read_ok
        jumpe 02,fail_nodsk
        move 05,member_mask
        jumpe 05,fail_noset
        andcm 05,found_mask
        jumpn 05,fail_noset

stage1_have_set:
        setzm stream_member
        pushj 017,read_next_stream_sector
        jumpe 01,fail_khead

        move 02,buffer
        came 02,daimon_magic
        jrst fail_khead
stage1_magic_ok:
        hlrz 03,buffer+000001
        jumpe 03,fail_layout
        hrrz 04,buffer+000001
        caml 04,03
        jrst fail_layout
        caile 03,020000
        jrst fail_layout
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
        pushj 017,parse_db0_badmap
        jumpe 01,locate_scan_next
        move 02,db1_next_sector
        jumpe 02,locate_no_db1
locate_db1_loop:
        move 02,db1_next_sector
        movem 02,last_badmap_sector
        pushj 017,unit_sector_to_dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        pushj 017,parse_db1_badmap
        jumpe 01,locate_scan_next
        move 02,db1_next_sector
        jumpe 02,locate_no_db1
        jrst locate_db1_loop
locate_no_db1:
        aos 02,last_badmap_sector
        movem 02,candidate_dbx
locate_dbx_skip_loop:
        move 02,candidate_dbx
        pushj 017,bad_contains_candidate
        jumpe 01,locate_dbx_try
        aos candidate_dbx
        jrst locate_dbx_skip_loop
locate_dbx_try:
        move 02,candidate_dbx
        pushj 017,unit_sector_to_dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        hlrz 02,buffer
        caie 02,0444270
        jrst locate_scan_next
locate_dbx_magic_ok:
        move 02,candidate_dbx
locate_descriptor:
        movem 02,located_dboot_loc
        pushj 017,parse_descriptor
        popj 017,

parse_db0_badmap:
        setzm bad_count
        hrrz 03,buffer
        move 01,03
        andi 01,0770002
        caie 01,010000
        jrst parse_db0_fail
        move 06,03
        lsh 06,-05
        andi 06,0177
        caile 06,020
        jrst parse_db0_fail
        setzm db1_next_sector
        trnn 03,000004
        jrst parse_db0_copy
parse_db0_has_db1:
        hrrz 04,buffer+000021
        movem 04,db1_next_sector
parse_db0_copy:
        movei 05,buffer+000001
        pushj 017,copy_bad_words
        jumpe 01,parse_db0_fail
parse_db0_done:
        move 02,scan_sector
        movem 02,last_badmap_sector
        jrst return_one
parse_db0_fail:
        jrst return_zero

parse_db1_badmap:
        hrrz 03,buffer
        hrrz 04,buffer+000001
        movem 04,db1_next_sector
        move 06,03
        lsh 06,-05
        andi 06,0177
        movei 05,buffer+000002
        pushj 017,copy_bad_words
        jumpe 01,parse_db1_fail
parse_db1_done:
        jrst return_one
parse_db1_fail:
        jrst return_zero

copy_bad_words:
        jumpe 06,copy_bad_success
load_bad_words_loop:
        move 02,0(05)
        pushj 017,append_bad_run_word
        jumpe 01,copy_bad_return
        aoj 05,
        sojg 06,load_bad_words_loop
copy_bad_success:
        jrst return_one
copy_bad_return:
        popj 017,

append_bad_run_word:
        move 07,bad_count
        cail 07,0177
        jrst return_zero
append_bad_room:
        hlrz 04,02
        movem 04,bad_start(07)
        move 03,02
        lsh 03,-06
        andi 03,07777
        addi 03,01
        add 03,04
        movem 03,bad_end(07)
        aos bad_count
        jrst return_one

bad_contains_candidate:
        movei 05,0
bad_contains_loop:
        caml 05,bad_count
        jrst bad_contains_no
        move 03,candidate_dbx
        camge 03,bad_start(05)
        jrst bad_contains_next
        caml 03,bad_end(05)
        jrst bad_contains_next
        jrst return_one
bad_contains_next:
        aoja 05,bad_contains_loop
bad_contains_no:
        jrst return_zero

parse_descriptor:
        hrrz 01,buffer
        trne 01,0770037
        jrst return_zero
        move 02,buffer+000002
	; generation zero only
        jumpn 02,return_zero
        move 02,buffer+000005
        move 03,02
        lsh 03,-020
        andi 03,017
        cail 03,04
        jrst return_zero
parse_desc_index_ok:
        move 04,02
        lsh 04,-024
        jumpe 04,return_zero
        move 05,02
        lsh 05,-014
        andi 05,017
        jumpe 05,return_zero
        caile 05,04
        jrst return_zero
        caml 03,05
        jrst return_zero
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
        movei 01,01
        lsh 01,0(05)
        subi 01,01
        came 01,04
        jrst return_zero

parse_desc_store:
        move 05,current_unit
        movem 05,member_unit(03)
        move 05,located_dboot_loc
        aoj 05,
        movem 05,member_next_sector(03)
        move 05,bad_count
        movem 05,member_bad_count(03)
        move 06,03
        lsh 06,07
        movei 05,0
copy_member_bad_loop:
        caml 05,bad_count
        jrst copy_bad_done
        move 01,bad_start(05)
        movem 01,member_bad_start(06)
        move 01,bad_end(05)
        movem 01,member_bad_end(06)
        aoj 05,
        aoja 06,copy_member_bad_loop
copy_bad_done:
        movei 01,01
        popj 017,
locate_dbc:
        skipn 02,scan_sector
        jrst locate_descriptor
return_zero:
        setz 01,
        popj 017,

read_next_stream_sector:
        move 06,stream_member
        move 02,member_next_sector(06)
        pushj 017,skip_member_bad_sectors
        movem 02,member_next_sector(06)
        aos member_next_sector(06)
        move 05,member_unit(06)
        pushj 017,linear_to_dsk_addr
        lsh 05,020
        add 02,05
        pushj 017,read_dsk_sector
        jumpe 01,read_next_done
        aos 03,stream_member
        caml 03,member_count
        setzm stream_member
read_next_done:
        popj 017,

skip_member_bad_sectors:
skip_bad_restart:
        move 03,06
        lsh 03,07
        move 04,member_bad_count(06)
skip_bad_loop:
        jumpe 04,skip_bad_done
        camge 02,member_bad_start(03)
        jrst skip_bad_next
        caml 02,member_bad_end(03)
        jrst skip_bad_next
        move 02,member_bad_end(03)
        jrst skip_bad_restart
skip_bad_next:
        aoj 03,
        sojg 04,skip_bad_loop
skip_bad_done:
        popj 017,

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
        jumpe 01,read_dsk_fail_end
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
        movei 04,0200000
        jrst wait_dsk_status

wait_ids:
        movei 03,0400000
        movei 04,0400000
        jrst wait_dsk_status

wait_dsk_status:
        coni 0270,05
        trne 05,01777
        jrst wait_fail
        and 05,03
        jumpn 05,wait_success
        sojg 04,wait_dsk_status
        jrst return_zero

wait_dct_rq:
        movei 04,0400000
wait_dct_loop:
        coni 0200,05
        trne 05,01000
        jrst wait_success
        sojg 04,wait_dct_loop
        jrst return_zero
wait_success:
return_one:
        movei 01,01
        popj 017,
wait_fail:
        jrst return_zero

fail_nodsk:
        movei 01,000001
        jrst fail_common
fail_noset:
        movei 01,000002
        jrst fail_common
fail_khead:
        movei 01,000003
        jrst fail_common
fail_layout:
        movei 01,000004
        jrst fail_common
fail_read:
        movei 01,000005
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
msg_layout:   .word 0375441715764
msg_read:     .word 0376245414400
        .bss
any_read_ok: .block 01
current_unit: .block 01
scan_sector: .block 01
found_mask: .block 01
member_mask: .block 01
member_count: .block 01
located_dboot_loc: .block 01
bad_count: .block 01
db1_next_sector: .block 01
last_badmap_sector: .block 01
candidate_dbx: .block 01
stream_member: .block 01
entry_addr: .block 01
stage1_last_error: .block 01
member_unit: .block 04
member_bad_count: .block 04
member_next_sector: .block 04
bad_start: .block 0177
bad_end: .block 0177
member_bad_start: .block 01000
member_bad_end: .block 01000
buffer: .block 0200
stage1_bss_end:
