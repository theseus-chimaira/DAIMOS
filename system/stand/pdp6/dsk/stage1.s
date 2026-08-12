; pdp6_stage1_hdd_v1.s -- pure assembler PDP-6 HDD/DBOOT V1 Stage1.
;
; This is the paper-tape/RIM Stage1 image.  It scans DSK270 units 0..3,
; accepts compact DBC or DB0/DB1/DBX metadata, reconstructs a round-robin
; opaque image stream across one to four members, skips each member's bad-run
; table, loads the image at 040000, and jumps to its relative entry point.

        .text
        .globl __start
        .globl pdp6_stage1_hdd_v1_start
        .globl start

__start:
pdp6_stage1_hdd_v1_start:
start:
        movei 017,073040
        pushj 017,stage1_clear_state

        setzm current_unit
stage1_unit_loop:
        move 02,current_unit
        caige 02,04
        jrst stage1_try_unit
        jrst stage1_units_done
stage1_try_unit:
        pushj 017,locate_unit
        jumpe 01,stage1_next_unit
        move 02,located_index
        move 03,bit_table(02)
        move 04,found_mask
        ior 04,03
        movem 04,found_mask
        move 05,member_mask
        and 04,05
        camn 04,member_mask
        jrst stage1_units_done
stage1_next_unit:
        aos current_unit
        jrst stage1_unit_loop

stage1_units_done:
        move 02,any_read_ok
        jumpe 02,fail_nodsk
        move 02,first_desc_seen
        jumpe 02,fail_noset
        move 02,found_mask
        and 02,member_mask
        camn 02,member_mask
        jrst stage1_have_set
        jrst fail_noset

stage1_have_set:
        pushj 017,validate_complete_set
        jumpe 01,fail_noset
        pushj 017,init_stream_cursors
        pushj 017,read_next_stream_sector
        jumpe 01,fail_khead

        move 02,buffer
        camn 02,daimon_magic
        jrst stage1_magic_ok
        jrst fail_khead
stage1_magic_ok:
        hlrz 03,buffer+000001
        jumpe 03,fail_layout
        hrrz 04,buffer+000001
        caml 04,03
        jrst fail_layout
        movei 05,000002
        add 05,03
        movem 05,total_stream_words
        pushj 017,compute_total_sectors

        movei 05,040000
        add 05,03
        caile 05,060000
        jrst fail_layout
        movei 05,040000
        add 05,04
        movem 05,entry_addr

        setzm stream_sector
        pushj 017,copy_stream_sector
        aos stream_sector
load_image_loop:
        move 02,total_stream_sectors
        sub 02,stream_sector
        jumple 02,load_image_done
        pushj 017,read_next_stream_sector
        jumpe 01,fail_read
        pushj 017,copy_stream_sector
        aos stream_sector
        jrst load_image_loop
load_image_done:
        movei 017,050000
        setz 01,
        move 02,member_count
        jrst @entry_addr

stage1_clear_state:
        setzm any_read_ok
        setzm first_desc_seen
        setzm found_mask
        setzm member_mask
        setzm member_count
        movei 01,0
clear_member_loop:
        caige 01,04
        jrst clear_member_one
        popj 017,
clear_member_one:
        setzm member_unit(01)
        setzm member_dboot_loc(01)
        setzm member_bad_count(01)
        setzm member_next_sector(01)
        aoj 01,
        jrst clear_member_loop

; Locate DBOOT on current_unit.  AC1 = 1 if a usable descriptor was stored.
locate_unit:
        setzm scan_sector
locate_scan_loop:
        move 02,scan_sector
        caige 02,0200
        jrst locate_scan_try
        setz 01,
        popj 017,
locate_scan_try:
        move 02,scan_sector
        pushj 017,unit_sector_to_dsk_addr
        movem 02,dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        movei 02,01
        movem 02,any_read_ok
        hlrz 02,buffer
        camn 02,magic_dbc
        jrst locate_dbc
        camn 02,magic_db0
        jrst locate_db0
locate_scan_next:
        aos scan_sector
        jrst locate_scan_loop

locate_dbc:
        move 02,scan_sector
        movem 02,located_dboot_loc
        pushj 017,parse_descriptor
        popj 017,

locate_db0:
        move 02,scan_sector
        movem 02,located_dboot_loc
        pushj 017,parse_db0_badmap
        jumpe 01,locate_scan_next
        move 02,db1_next_sector
        jumpe 02,locate_no_db1
locate_db1_loop:
        move 02,db1_next_sector
        movem 02,last_badmap_sector
        pushj 017,unit_sector_to_dsk_addr
        movem 02,dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        hlrz 02,buffer
        camn 02,magic_db1
        jrst locate_db1_magic_ok
        jrst locate_scan_next
locate_db1_magic_ok:
        pushj 017,parse_db1_badmap
        jumpe 01,locate_scan_next
        move 02,db1_next_sector
        jumpe 02,locate_no_db1
        jrst locate_db1_loop
locate_no_db1:
        move 02,last_badmap_sector
        addi 02,01
        movem 02,candidate_dbx
locate_dbx_skip_loop:
        move 02,candidate_dbx
        pushj 017,bad_contains_candidate
        jumpe 01,locate_dbx_try
        aos candidate_dbx
        movei 02,01
        jrst locate_dbx_skip_loop
locate_dbx_try:
        move 02,candidate_dbx
        pushj 017,unit_sector_to_dsk_addr
        movem 02,dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,locate_scan_next
        hlrz 02,buffer
        camn 02,magic_dbx
        jrst locate_dbx_magic_ok
        jrst locate_scan_next
locate_dbx_magic_ok:
        move 02,candidate_dbx
        movem 02,located_dboot_loc
        pushj 017,parse_descriptor
        popj 017,

parse_db0_badmap:
        setzm bad_count
        move 02,buffer
        pushj 017,header_version
        caie 01,01
        jrst parse_db0_fail
        move 02,buffer
        pushj 017,header_count
        caile 01,020
        jrst parse_db0_fail
        movem 01,bad_copy_count
        move 02,buffer
        pushj 017,header_flags
        move 03,01
        trne 03,000002
        jrst parse_db0_fail
        setzm db1_next_sector
        trne 03,000004
        jrst parse_db0_has_db1
        jrst parse_db0_copy
parse_db0_has_db1:
        hrrz 04,buffer+000021
        jumpe 04,parse_db0_fail
        movem 04,db1_next_sector
parse_db0_copy:
        movei 05,0
parse_db0_copy_loop:
        move 06,bad_copy_count
        sub 06,05
        jumple 06,parse_db0_done
        move 02,buffer+000001(05)
        pushj 017,append_bad_run_word
        jumpe 01,parse_db0_fail
        aoj 05,
        jrst parse_db0_copy_loop
parse_db0_done:
        move 02,scan_sector
        movem 02,last_badmap_sector
        movei 01,01
        popj 017,
parse_db0_fail:
        setz 01,
        popj 017,

parse_db1_badmap:
        move 02,buffer
        pushj 017,header_version
        caie 01,01
        jrst parse_db1_fail
        move 02,buffer
        pushj 017,header_flags
        jumpe 01,parse_db1_flags_ok
        jrst parse_db1_fail
parse_db1_flags_ok:
        hrrz 04,buffer+000001
        movem 04,db1_next_sector
        move 02,buffer
        pushj 017,header_count
        movem 01,bad_copy_count
        movei 05,0
parse_db1_copy_loop:
        move 06,bad_copy_count
        sub 06,05
        jumple 06,parse_db1_done
        move 07,bad_count
        caige 07,0177
        jrst parse_db1_room
        jrst parse_db1_fail
parse_db1_room:
        move 02,buffer+000002(05)
        pushj 017,append_bad_run_word
        jumpe 01,parse_db1_fail
        aoj 05,
        jrst parse_db1_copy_loop
parse_db1_done:
        movei 01,01
        popj 017,
parse_db1_fail:
        setz 01,
        popj 017,

append_bad_run_word:
        move 07,bad_count
        caige 07,0177
        jrst append_bad_room
        setz 01,
        popj 017,
append_bad_room:
        move 03,02
        hlrz 04,03
        movem 04,bad_start(07)
        move 05,03
        lsh 05,-06
        andi 05,07777
        addi 05,01
        add 05,04
        movem 05,bad_end(07)
        aos bad_count
        movei 01,01
        popj 017,

bad_contains_candidate:
        movei 05,0
bad_contains_loop:
        move 06,bad_count
        sub 06,05
        jumple 06,bad_contains_no
        move 03,candidate_dbx
        sub 03,bad_start(05)
        jumpl 03,bad_contains_next
        move 03,bad_end(05)
        sub 03,candidate_dbx
        jumple 03,bad_contains_next
        movei 01,01
        popj 017,
bad_contains_next:
        aoj 05,
        jrst bad_contains_loop
bad_contains_no:
        setz 01,
        popj 017,

parse_descriptor:
        move 02,buffer
        pushj 017,header_version
        caie 01,0
        jrst parse_desc_fail
        move 02,buffer
        pushj 017,header_flags
        jumpn 01,parse_desc_fail
        move 02,buffer+000002
        jumpn 02,parse_desc_fail          ; generation zero only

        move 02,buffer+000005
        move 03,02
        lsh 03,-020
        andi 03,017
        caige 03,04
        jrst parse_desc_index_ok
        jrst parse_desc_fail
parse_desc_index_ok:
        movem 03,located_index
        move 04,02
        lsh 04,-024
        andi 04,0177777
        jumpe 04,parse_desc_fail
        movem 04,located_mask
        move 05,02
        lsh 05,-014
        andi 05,017
        jumpe 05,parse_desc_fail
        caile 05,04
        jrst parse_desc_fail
        movem 05,located_count
        camge 03,05
        jrst parse_desc_member_ok
        jrst parse_desc_fail
parse_desc_member_ok:
        move 06,bit_table(03)
        move 07,04
        and 07,06
        jumpe 07,parse_desc_fail

        move 05,first_desc_seen
        jumpe 05,parse_desc_first
        move 05,member_mask
        camn 05,located_mask
        jrst parse_desc_mask_ok
        jrst parse_desc_fail
parse_desc_mask_ok:
        move 05,member_count
        camn 05,located_count
        jrst parse_desc_store
        jrst parse_desc_fail

parse_desc_first:
        movei 05,01
        movem 05,first_desc_seen
        move 05,located_mask
        movem 05,member_mask
        move 05,located_count
        movem 05,member_count
        pushj 017,expected_member_mask
        camn 01,member_mask
        jrst parse_desc_store
        jrst parse_desc_fail

parse_desc_store:
        move 03,located_index
        move 05,current_unit
        movem 05,member_unit(03)
        move 05,located_dboot_loc
        movem 05,member_dboot_loc(03)
        move 05,bad_count
        movem 05,member_bad_count(03)
        pushj 017,copy_bad_runs_to_member
        movei 01,01
        popj 017,
parse_desc_fail:
        setz 01,
        popj 017,

expected_member_mask:
        movei 01,01
        move 02,member_count
expected_mask_loop:
        sojle 02,expected_mask_done
        lsh 01,01
        ori 01,01
        jrst expected_mask_loop
expected_mask_done:
        popj 017,

copy_bad_runs_to_member:
        move 06,located_index
        lsh 06,07
        movei 05,0
copy_bad_loop:
        move 07,bad_count
        sub 07,05
        jumple 07,copy_bad_done
        move 01,bad_start(05)
        movem 01,member_bad_start(06)
        move 01,bad_end(05)
        movem 01,member_bad_end(06)
        aoj 05,
        aoj 06,
        jrst copy_bad_loop
copy_bad_done:
        popj 017,

header_version:
        lsh 02,-014
        andi 02,077
        move 01,02
        popj 017,
header_count:
        lsh 02,-05
        andi 02,0177
        move 01,02
        popj 017,
header_flags:
        andi 02,037
        move 01,02
        popj 017,

validate_complete_set:
        pushj 017,expected_member_mask
        camn 01,member_mask
        jrst validate_mask_ok
        setz 01,
        popj 017,
validate_mask_ok:
        move 02,found_mask
        and 02,member_mask
        camn 02,member_mask
        jrst validate_set_ok
        setz 01,
        popj 017,
validate_set_ok:
        movei 01,01
        popj 017,

init_stream_cursors:
        setzm stream_member
        movei 03,0
init_cursor_loop:
        move 04,member_count
        sub 04,03
        jumple 04,init_cursor_done
        move 05,member_dboot_loc(03)
        aoj 05,
        movem 05,member_next_sector(03)
        aoj 03,
        jrst init_cursor_loop
init_cursor_done:
        popj 017,

read_next_stream_sector:
        move 06,stream_member
        move 02,member_next_sector(06)
        pushj 017,skip_member_bad_sectors
        movem 02,member_next_sector(06)
        move 03,02
        aoj 03,
        movem 03,member_next_sector(06)
        move 05,member_unit(06)
        pushj 017,linear_to_dsk_addr
        lsh 05,020
        add 02,05
        movem 02,dsk_addr
        pushj 017,read_dsk_sector
        jumpe 01,read_next_done
        aos stream_member
        move 03,stream_member
        caml 03,member_count
        setzm stream_member
read_next_done:
        popj 017,

skip_member_bad_sectors:
        move 03,06
        lsh 03,07
        move 04,member_bad_count(06)
        movei 05,0
skip_bad_restart:
        move 07,04
        sub 07,05
        jumple 07,skip_bad_done
        move 01,02
        sub 01,member_bad_start(03)
        jumpl 01,skip_bad_next
        move 01,member_bad_end(03)
        sub 01,02
        jumple 01,skip_bad_next
        move 02,member_bad_end(03)
        movei 05,0
        move 03,06
        lsh 03,07
        jrst skip_bad_restart
skip_bad_next:
        aoj 03,
        aoj 05,
        jrst skip_bad_restart
skip_bad_done:
        popj 017,

compute_total_sectors:
        move 02,total_stream_words
        movei 03,0
compute_total_sectors_loop:
        jumple 02,compute_total_sectors_done
        aoj 03,
        subi 02,0200
        jrst compute_total_sectors_loop
compute_total_sectors_done:
        movem 03,total_stream_sectors
        popj 017,

copy_stream_sector:
        move 06,stream_sector
        lsh 06,07
        movei 07,0
copy_stream_loop:
        caml 06,total_stream_words
        jrst copy_stream_done
        caige 06,000002
        jrst copy_stream_advance
        movei 01,037776
        add 01,06
        move 04,buffer(07)
        movem 04,0(01)
copy_stream_advance:
        aoj 06,
        aoj 07,
        caige 07,0200
        jrst copy_stream_loop
copy_stream_done:
        popj 017,

unit_sector_to_dsk_addr:
        pushj 017,linear_to_dsk_addr
        move 05,current_unit
        lsh 05,020
        add 02,05
        popj 017,

linear_to_dsk_addr:
        movei 03,0
linear_to_dsk_loop:
        caige 02,000054
        jrst linear_to_dsk_done
        subi 02,000054
        aoj 03,
        jrst linear_to_dsk_loop
linear_to_dsk_done:
        lsh 03,06
        add 02,03
        popj 017,

read_dsk_sector:
        cono 0200,04000
        datao 0270,dsk_addr
        pushj 017,wait_dfr
        jumpe 01,read_dsk_fail
        cono 0270,01000
        movei 02,0
read_loop:
        pushj 017,wait_dct_rq
        jumpe 01,read_dsk_fail_end
        datai 0200,ioword
        move 03,ioword
        movem 03,buffer(02)
        aoj 02,
        caige 02,0200
        jrst read_loop
        cono 0270,030000
        pushj 017,wait_ids
        jumpe 01,read_dsk_fail
        movei 01,01
        popj 017,
read_dsk_fail_end:
        cono 0270,030000
read_dsk_fail:
        cono 0270,030000
        setz 01,
        popj 017,

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
        move 06,05
        and 06,03
        jumpn 06,wait_success
        sojg 04,wait_dsk_status
        setz 01,
        popj 017,

wait_dct_rq:
        movei 04,0400000
wait_dct_loop:
        coni 0200,05
        trne 05,01000
        jrst wait_success
        sojg 04,wait_dct_loop
        setz 01,
        popj 017,
wait_success:
        movei 01,01
        popj 017,
wait_fail:
        setz 01,
        popj 017,

fail_nodsk:
        move 01,msg_nodsk
        pushj 017,put_sixbit_word
        movei 01,000001
        jrst halt_stage1
fail_noset:
        move 01,msg_noset
        pushj 017,put_sixbit_word
        movei 01,000002
        jrst halt_stage1
fail_khead:
        move 01,msg_khead
        pushj 017,put_sixbit_word
        movei 01,000003
        jrst halt_stage1
fail_layout:
        move 01,msg_layout
        pushj 017,put_sixbit_word
        movei 01,000004
        jrst halt_stage1
fail_read:
        move 01,msg_read
        pushj 017,put_sixbit_word
        movei 01,000005
        jrst halt_stage1
halt_stage1:
        movem 01,stage1_last_error
        halt .
        jrst halt_stage1

put_sixbit_word:
        movem 01,put_word
        movei 06,0
put_six_loop:
        caige 06,06
        jrst put_six_one
put_six_final_wait:
        coni 0120,cty_status
        move 04,cty_status
        trne 04,0020
        jrst put_six_final_wait
        popj 017,
put_six_one:
        move 02,put_word
        move 03,put_shift(06)
        lsh 02,0(03)
        andi 02,077
        addi 02,040
        movem 02,ioword
put_wait:
        coni 0120,cty_status
        move 04,cty_status
        trne 04,0020
        jrst put_wait
        datao 0120,ioword
        aoj 06,
        jrst put_six_loop

magic_db0:    .word 0444220
magic_db1:    .word 0444221
magic_dbx:    .word 0444270
magic_dbc:    .word 0444243
daimon_magic: .word 0444151555756
msg_nodsk:    .word 0375657446353
msg_noset:    .word 0375657634564
msg_khead:    .word 0375350454144
msg_layout:   .word 0374441715764
msg_read:     .word 0375245414400
bit_table:    .word 01
              .word 02
              .word 04
              .word 010
put_shift:    .word -036
              .word -030
              .word -022
              .word -014
              .word -06
              .word 0

        .bss
any_read_ok: .block 01
first_desc_seen: .block 01
current_unit: .block 01
scan_sector: .block 01
found_mask: .block 01
member_mask: .block 01
member_count: .block 01
located_dboot_loc: .block 01
located_index: .block 01
located_mask: .block 01
located_count: .block 01
bad_count: .block 01
bad_copy_count: .block 01
db1_next_sector: .block 01
last_badmap_sector: .block 01
candidate_dbx: .block 01
total_stream_words: .block 01
total_stream_sectors: .block 01
stream_sector: .block 01
stream_member: .block 01
entry_addr: .block 01
stage1_last_error: .block 01
put_word: .block 01
dsk_addr: .block 01
ioword: .block 01
cty_status: .block 01
member_unit: .block 04
member_dboot_loc: .block 04
member_bad_count: .block 04
member_next_sector: .block 04
bad_start: .block 0177
bad_end: .block 0177
member_bad_start: .block 01000
member_bad_end: .block 01000
buffer: .block 0200
stage1_bss_end:
