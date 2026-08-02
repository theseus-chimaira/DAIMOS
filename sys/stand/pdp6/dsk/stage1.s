; pdp6_stage1_hdd_v1.s -- pure assembler PDP-6 HDD/DBOOT V1 Stage1.
;
; This is the paper-tape/RIM Stage1 image.  It scans DSK270 units 0..3,
; accepts compact DBC or DB0/DB1/DBX metadata, loads the DAIMON split stream,
; builds BOOTINFO at 073000, and jumps to KINIT with AC1 = BOOTINFO.
; Stage1 remains a small loader with no higher-level policy.

        .text
        .globl __start
        .globl pdp6_stage1_hdd_v1_start
        .globl start

__start:
pdp6_stage1_hdd_v1_start:
start:
        movei 17,073040
        pushj 17,stage1_clear_state

        setzm current_unit
stage1_unit_loop:
        move 2,current_unit
        caige 2,4
        jrst stage1_try_unit
        jrst stage1_units_done
stage1_try_unit:
        pushj 17,locate_unit
        jumpe 1,stage1_next_unit
        move 2,located_index
        move 3,bit_table(2)
        move 4,found_mask
        ior 4,3
        movem 4,found_mask
        move 5,member_mask
        and 4,5
        camn 4,member_mask
        jrst stage1_units_done
stage1_next_unit:
        aos current_unit
        jrst stage1_unit_loop

stage1_units_done:
        move 2,any_read_ok
        jumpe 2,fail_nodsk
        move 2,first_desc_seen
        jumpe 2,fail_noset
        move 2,found_mask
        and 2,member_mask
        camn 2,member_mask
        jrst stage1_have_set
        jrst fail_noset

stage1_have_set:
        pushj 17,build_initial_bootinfo
        setzm stream_sector
        movei 2,0
        pushj 17,kernel_logical_to_dsk_addr
        jumpe 1,fail_read
        movem 2,dsk_addr
        pushj 17,read_dsk_sector
        jumpe 1,fail_khead

        move 2,buffer
        camn 2,daimon_magic
        jrst stage1_magic_ok
        jrst fail_khead
stage1_magic_ok:
        hlrz 3,buffer+000001
        movem 3,kcore_file_words
        movem 3,kcore_memory_words
        hrrz 3,buffer+000001
        movem 3,kinit_file_words
        movem 3,kinit_memory_words
        hlrz 3,buffer+000002
        movem 3,kinit_entry_offset

        movei 3,000003
        add 3,kcore_file_words
        movem 3,kinit_file_start
        add 3,kinit_file_words
        movem 3,total_file_words
        pushj 17,compute_total_sectors

        movei 1,073000
        sub 1,kinit_memory_words
        movem 1,kinit_start
        movei 2,000040
        add 2,kcore_memory_words
        movem 2,kinit_stack_base
        move 3,kinit_start
        sub 3,kinit_stack_base
        caige 3,000200
        jrst fail_layout
        pushj 17,build_loaded_bootinfo

        setzm stream_sector
load_kernel_loop:
        move 2,total_kernel_sectors
        sub 2,stream_sector
        jumple 2,load_kernel_done
        move 2,stream_sector
        pushj 17,kernel_logical_to_dsk_addr
        jumpe 1,fail_read
        movem 2,dsk_addr
        pushj 17,read_dsk_sector
        jumpe 1,fail_read
        pushj 17,copy_stream_sector
        aos stream_sector
        jrst load_kernel_loop
load_kernel_done:
        ; KINIT copies INITFS above the fixed KCORE stack area before fixed
        ; MRES staging.  Its temporary stack therefore needs space after
        ; KCORE, not after the future resident INITFS image.
        movei 2,000040
        add 2,kcore_memory_words
        movem 2,kinit_stack_base
        move 3,kinit_start
        sub 3,kinit_stack_base
        caige 3,000200
        jrst fail_layout

        move 2,kinit_start
        add 2,kinit_entry_offset
        movem 2,kinit_entry_addr
        move 17,kinit_stack_base
        movei 1,073000
        jrst @kinit_entry_addr

stage1_clear_state:
        setzm any_read_ok
        setzm first_desc_seen
        setzm found_mask
        setzm member_mask
        setzm member_count
        movei 1,0
clear_member_loop:
        caige 1,4
        jrst clear_member_one
        popj 17,
clear_member_one:
        setzm member_unit(1)
        setzm member_boot_start(1)
        setzm member_boot_count(1)
        setzm member_dboot_loc(1)
        aoj 1,
        jrst clear_member_loop

; Locate DBOOT on current_unit.  AC1 = 1 if a usable descriptor was stored.
locate_unit:
        setzm scan_sector
locate_scan_loop:
        move 2,scan_sector
        caige 2,0200
        jrst locate_scan_try
        setz 1,
        popj 17,
locate_scan_try:
        move 2,scan_sector
        pushj 17,unit_sector_to_dsk_addr
        movem 2,dsk_addr
        pushj 17,read_dsk_sector
        jumpe 1,locate_scan_next
        movei 2,1
        movem 2,any_read_ok
        hlrz 2,buffer
        camn 2,magic_dbc
        jrst locate_dbc
        camn 2,magic_db0
        jrst locate_db0
locate_scan_next:
        aos scan_sector
        jrst locate_scan_loop

locate_dbc:
        move 2,scan_sector
        movem 2,located_dboot_loc
        pushj 17,parse_descriptor
        popj 17,

locate_db0:
        move 2,scan_sector
        movem 2,located_dboot_loc
        pushj 17,parse_db0_badmap
        jumpe 1,locate_scan_next
        move 2,db1_next_sector
        jumpe 2,locate_no_db1
locate_db1_loop:
        move 2,db1_next_sector
        movem 2,last_badmap_sector
        pushj 17,unit_sector_to_dsk_addr
        movem 2,dsk_addr
        pushj 17,read_dsk_sector
        jumpe 1,locate_scan_next
        hlrz 2,buffer
        camn 2,magic_db1
        jrst locate_db1_magic_ok
        jrst locate_scan_next
locate_db1_magic_ok:
        pushj 17,parse_db1_badmap
        jumpe 1,locate_scan_next
        move 2,db1_next_sector
        jumpe 2,locate_no_db1
        jrst locate_db1_loop
locate_no_db1:
        move 2,last_badmap_sector
        addi 2,1
        movem 2,candidate_dbx
locate_dbx_skip_loop:
        move 2,candidate_dbx
        pushj 17,bad_contains_candidate
        jumpe 1,locate_dbx_try
        aos candidate_dbx
        movei 2,1
        jrst locate_dbx_skip_loop
locate_dbx_try:
        move 2,candidate_dbx
        pushj 17,unit_sector_to_dsk_addr
        movem 2,dsk_addr
        pushj 17,read_dsk_sector
        jumpe 1,locate_scan_next
        hlrz 2,buffer
        camn 2,magic_dbx
        jrst locate_dbx_magic_ok
        jrst locate_scan_next
locate_dbx_magic_ok:
        pushj 17,parse_descriptor
        popj 17,

parse_db0_badmap:
        setzm bad_count
        move 2,buffer
        pushj 17,header_version
        caie 1,1
        jrst parse_db0_fail
        move 2,buffer
        pushj 17,header_count
        caile 1,020
        jrst parse_db0_fail
        movem 1,bad_copy_count
        move 2,buffer
        pushj 17,header_flags
        move 3,1
        trne 3,000002
        jrst parse_db0_fail
        setzm db1_next_sector
        trne 3,000004
        jrst parse_db0_has_db1
        jrst parse_db0_copy
parse_db0_has_db1:
        hrrz 4,buffer+000021
        jumpe 4,parse_db0_fail
        movem 4,db1_next_sector
parse_db0_copy:
        movei 5,0
parse_db0_copy_loop:
        move 6,bad_copy_count
        sub 6,5
        jumple 6,parse_db0_done
        move 2,buffer+000001(5)
        pushj 17,append_bad_run_word
        jumpe 1,parse_db0_fail
        aoj 5,
        jrst parse_db0_copy_loop
parse_db0_done:
        move 2,scan_sector
        movem 2,last_badmap_sector
        movei 1,1
        popj 17,
parse_db0_fail:
        setz 1,
        popj 17,

parse_db1_badmap:
        move 2,buffer
        pushj 17,header_version
        caie 1,1
        jrst parse_db1_fail
        move 2,buffer
        pushj 17,header_flags
        jumpe 1,parse_db1_flags_ok
        jrst parse_db1_fail
parse_db1_flags_ok:
        hrrz 4,buffer+000001
        movem 4,db1_next_sector
        move 2,buffer
        pushj 17,header_count
        movem 1,bad_copy_count
        movei 5,0
parse_db1_copy_loop:
        move 6,bad_copy_count
        sub 6,5
        jumple 6,parse_db1_done
        move 7,bad_count
        caige 7,0177
        jrst parse_db1_room
        jrst parse_db1_fail
parse_db1_room:
        move 2,buffer+000002(5)
        pushj 17,append_bad_run_word
        jumpe 1,parse_db1_fail
        aoj 5,
        jrst parse_db1_copy_loop
parse_db1_done:
        movei 1,1
        popj 17,
parse_db1_fail:
        setz 1,
        popj 17,

append_bad_run_word:
        move 7,bad_count
        caige 7,0177
        jrst append_bad_room
        setz 1,
        popj 17,
append_bad_room:
        move 3,2
        hlrz 4,3
        movem 4,bad_start(7)
        move 5,3
        lsh 5,-6
        andi 5,07777
        addi 5,1
        add 5,4
        movem 5,bad_end(7)
        aos bad_count
        movei 1,1
        popj 17,

bad_contains_candidate:
        movei 5,0
bad_contains_loop:
        move 6,bad_count
        sub 6,5
        jumple 6,bad_contains_no
        move 3,candidate_dbx
        sub 3,bad_start(5)
        jumpl 3,bad_contains_next
        move 3,bad_end(5)
        sub 3,candidate_dbx
        jumple 3,bad_contains_next
        movei 1,1
        popj 17,
bad_contains_next:
        aoj 5,
        jrst bad_contains_loop
bad_contains_no:
        setz 1,
        popj 17,

parse_descriptor:
        move 2,buffer
        pushj 17,header_version
        caie 1,0
        jrst parse_desc_fail
        move 2,buffer+000005
        move 3,2
        lsh 3,-20
        andi 3,017
        caige 3,4
        jrst parse_desc_index_ok
        jrst parse_desc_fail
parse_desc_index_ok:
        movem 3,located_index
        move 4,2
        lsh 4,-24
        andi 4,0177777
        jumpe 4,parse_desc_fail
        movem 4,located_mask
        hlrz 5,buffer+000012
        jumpe 5,parse_desc_fail
        movem 5,located_boot_start
        hrrz 5,buffer+000012
        jumpe 5,parse_desc_fail
        movem 5,located_boot_count

        move 5,first_desc_seen
        jumpe 5,parse_desc_first
        move 5,expected_uuid0
        camn 5,buffer+000003
        jrst parse_desc_uuid0_ok
        jrst parse_desc_fail
parse_desc_uuid0_ok:
        move 5,expected_uuid1
        camn 5,buffer+000004
        jrst parse_desc_uuid1_ok
        jrst parse_desc_fail
parse_desc_uuid1_ok:
        move 5,expected_generation
        camn 5,buffer+000002
        jrst parse_desc_gen_ok
        jrst parse_desc_fail
parse_desc_gen_ok:
        move 5,member_mask
        camn 5,located_mask
        jrst parse_desc_store
        jrst parse_desc_fail

parse_desc_first:
        movei 5,1
        movem 5,first_desc_seen
        move 5,buffer+000003
        movem 5,expected_uuid0
        move 5,buffer+000004
        movem 5,expected_uuid1
        move 5,buffer+000002
        movem 5,expected_generation
        move 5,located_mask
        movem 5,member_mask
        pushj 17,popcount_member_mask
        jumpe 1,parse_desc_fail
        caile 1,4
        jrst parse_desc_fail
        movem 1,member_count

parse_desc_store:
        move 3,located_index
        move 5,current_unit
        movem 5,member_unit(3)
        move 5,located_boot_start
        movem 5,member_boot_start(3)
        move 5,located_boot_count
        movem 5,member_boot_count(3)
        move 5,located_dboot_loc
        movem 5,member_dboot_loc(3)
        movei 1,1
        popj 17,
parse_desc_fail:
        setz 1,
        popj 17,

header_version:
        lsh 2,-14
        andi 2,077
        move 1,2
        popj 17,
header_count:
        lsh 2,-5
        andi 2,0177
        move 1,2
        popj 17,
header_flags:
        andi 2,037
        move 1,2
        popj 17,

popcount_member_mask:
        movei 1,0
        movei 5,0
popcount_loop:
        caige 5,4
        jrst popcount_one
        popj 17,
popcount_one:
        move 6,bit_table(5)
        move 7,member_mask
        and 7,6
        jumpe 7,popcount_next
        aoj 1,
popcount_next:
        aoj 5,
        jrst popcount_loop

build_initial_bootinfo:
        movei 1,073000
        movei 2,000035
        pushj 17,zero_words
        move 2,bi2_header
        movem 2,073000
        movei 2,000035
        movem 2,073001
        move 2,member_unit
        addi 2,001340             ; DSK270 I/O device field 056, unit low 4
        movem 2,073003
        move 2,expected_uuid0
        movem 2,073004
        move 2,expected_uuid1
        movem 2,073005
        move 2,expected_generation
        movem 2,073007
        move 2,member_mask
        lsh 2,24                  ; mask << 20 decimal
        move 3,member_count
        lsh 3,17                  ; count << 15 decimal
        ior 2,3
        movem 2,073010
        move 2,member_unit
        movem 2,073011
        move 2,member_unit+1
        lsh 2,4
        iorm 2,073011
        move 2,member_unit+2
        lsh 2,10
        iorm 2,073011
        move 2,member_unit+3
        lsh 2,14
        iorm 2,073011
        move 2,member_count
        lsh 2,34
        movem 2,073006
        move 2,member_dboot_loc
        lsh 2,22                  ; loc0 << 18 decimal
        move 3,member_dboot_loc+1
        ior 2,3
        movem 2,073013
        move 2,member_dboot_loc+2
        lsh 2,22
        move 3,member_dboot_loc+3
        ior 2,3
        movem 2,073014
        popj 17,

build_loaded_bootinfo:
        movei 2,000040
        movem 2,073023
        move 2,kcore_file_words
        movem 2,073024
        move 2,kcore_memory_words
        movem 2,073025
        move 2,kinit_start
        add 2,kinit_entry_offset
        movem 2,073026
        move 2,kinit_start
        movem 2,073027
        move 2,kinit_file_words
        movem 2,073030
        move 2,kinit_memory_words
        movem 2,073031
        move 2,kinit_entry_offset
        movem 2,073032
        popj 17,

compute_total_sectors:
        move 2,total_file_words
        movei 3,0
compute_total_sectors_loop:
        jumple 2,compute_total_sectors_done
        aoj 3,
        subi 2,0200
        jrst compute_total_sectors_loop
compute_total_sectors_done:
        movem 3,total_kernel_sectors
        popj 17,

copy_stream_sector:
        move 6,stream_sector
        lsh 6,7
        movei 7,0
copy_stream_loop:
        caml 6,total_file_words
        jrst copy_stream_done
        caige 6,000003
        jrst copy_stream_advance
        caml 6,kinit_file_start
        jrst copy_stream_kinit
        movei 1,000035
        add 1,6
        jrst copy_stream_store
copy_stream_kinit:
        move 1,kinit_start
        add 1,6
        sub 1,kinit_file_start
        jrst copy_stream_store
copy_stream_advance:
        aoj 6,
        aoj 7,
        caige 7,0200
        jrst copy_stream_loop
        jrst copy_stream_done
copy_stream_store:
        move 4,buffer(7)
        movem 4,0(1)
        aoj 6,
        aoj 7,
        caige 7,0200
        jrst copy_stream_loop
copy_stream_done:
        popj 17,

kernel_logical_to_dsk_addr:
        move 3,2
        movei 4,0
kernel_div_loop:
        move 5,member_count
        sub 5,3
        jumple 5,kernel_div_sub
        jrst kernel_div_done
kernel_div_sub:
        move 5,member_count
        sub 3,5
        aoj 4,
        jrst kernel_div_loop
kernel_div_done:
        move 5,member_boot_count(3)
        sub 5,4
        jumple 5,kernel_addr_fail
        move 5,member_boot_start(3)
        add 4,5
        move 2,4
        move 6,3
        pushj 17,linear_to_dsk_addr
        move 5,member_unit(6)
        lsh 5,20
        add 2,5
        movei 1,1
        popj 17,
kernel_addr_fail:
        setz 1,
        popj 17,

unit_sector_to_dsk_addr:
        pushj 17,linear_to_dsk_addr
        move 5,current_unit
        lsh 5,20
        add 2,5
        popj 17,

linear_to_dsk_addr:
        movei 3,0
linear_to_dsk_loop:
        caige 2,000054
        jrst linear_to_dsk_done
        subi 2,000054
        aoj 3,
        jrst linear_to_dsk_loop
linear_to_dsk_done:
        lsh 3,6
        add 2,3
        popj 17,

read_dsk_sector:
        cono 0200,04000
        datao 0270,dsk_addr
        pushj 17,wait_dfr
        jumpe 1,read_dsk_fail
        cono 0270,01000
        movei 2,0
read_loop:
        pushj 17,wait_dct_rq
        jumpe 1,read_dsk_fail_end
        datai 0200,ioword
        move 3,ioword
        movem 3,buffer(2)
        aoj 2,
        caige 2,0200
        jrst read_loop
        cono 0270,030000
        pushj 17,wait_ids
        jumpe 1,read_dsk_fail
        movei 1,1
        popj 17,
read_dsk_fail_end:
        cono 0270,030000
read_dsk_fail:
        cono 0270,030000
        setz 1,
        popj 17,

wait_dfr:
        movei 3,040000
        movei 4,0200000
        jrst wait_dsk_status

wait_ids:
        movei 3,0400000
        movei 4,0400000
        jrst wait_dsk_status

wait_dsk_status:
        coni 0270,5
        trne 5,01777
        jrst wait_fail
        move 6,5
        and 6,3
        jumpn 6,wait_success
        sojg 4,wait_dsk_status
        setz 1,
        popj 17,

wait_dct_rq:
        movei 4,0400000
wait_dct_loop:
        coni 0200,5
        trne 5,01000
        jrst wait_success
        sojg 4,wait_dct_loop
        setz 1,
        popj 17,
wait_success:
        movei 1,1
        popj 17,
wait_fail:
        setz 1,
        popj 17,

zero_words:
        jumpe 2,zero_done
zero_loop:
        setzm 0(1)
        aoj 1,
        sojg 2,zero_loop
zero_done:
        popj 17,

fail_nodsk:
        move 1,msg_nodsk
        pushj 17,put_sixbit_word
        movei 1,000001
        jrst halt_stage1
fail_noset:
        move 1,msg_noset
        pushj 17,put_sixbit_word
        movei 1,000002
        jrst halt_stage1
fail_khead:
        move 1,msg_khead
        pushj 17,put_sixbit_word
        movei 1,000003
        jrst halt_stage1
fail_layout:
        move 1,msg_layout
        pushj 17,put_sixbit_word
        movei 1,000004
        jrst halt_stage1
fail_read:
        move 1,msg_read
        pushj 17,put_sixbit_word
        movei 1,000005
        jrst halt_stage1
halt_stage1:
        movem 1,stage1_last_error
        halt .
        jrst halt_stage1

put_sixbit_word:
        movem 1,put_word
        movei 6,0
put_six_loop:
        caige 6,6
        jrst put_six_one
put_six_final_wait:
        coni 0120,cty_status
        move 4,cty_status
        trne 4,0020
        jrst put_six_final_wait
        popj 17,
put_six_one:
        move 2,put_word
        move 3,put_shift(6)
        lsh 2,0(3)
        andi 2,077
        addi 2,040
        movem 2,ioword
put_wait:
        coni 0120,cty_status
        move 4,cty_status
        trne 4,0020
        jrst put_wait
        datao 0120,ioword
        aoj 6,
        jrst put_six_loop

magic_db0:    .word 444220
magic_db1:    .word 444221
magic_dbx:    .word 444270
magic_dbc:    .word 444243
bi2_header:   .word 425122020000
daimon_magic: .word 444151555756
msg_nodsk:    .word 375657446353
msg_noset:    .word 375657634564
msg_khead:    .word 375350454144
msg_layout:   .word 374441715764
msg_read:     .word 375245414400
bit_table:    .word 1
              .word 2
              .word 4
              .word 10
put_shift:    .word -36
              .word -30
              .word -22
              .word -14
              .word -6
              .word 0

        .bss
any_read_ok: .block 1
first_desc_seen: .block 1
current_unit: .block 1
scan_sector: .block 1
found_mask: .block 1
member_mask: .block 1
member_count: .block 1
expected_uuid0: .block 1
expected_uuid1: .block 1
expected_generation: .block 1
located_dboot_loc: .block 1
located_index: .block 1
located_mask: .block 1
located_boot_start: .block 1
located_boot_count: .block 1
bad_count: .block 1
bad_copy_count: .block 1
db1_next_sector: .block 1
last_badmap_sector: .block 1
candidate_dbx: .block 1
kcore_file_words: .block 1
kcore_memory_words: .block 1
kinit_file_words: .block 1
kinit_memory_words: .block 1
kinit_entry_offset: .block 1
total_file_words: .block 1
kinit_file_start: .block 1
total_kernel_sectors: .block 1
stream_sector: .block 1
kinit_start: .block 1
kinit_stack_base: .block 1
kinit_entry_addr: .block 1
stage1_last_error: .block 1
put_word: .block 1
dsk_addr: .block 1
ioword: .block 1
cty_status: .block 1
member_unit: .block 4
member_boot_start: .block 4
member_boot_count: .block 4
member_dboot_loc: .block 4
bad_start: .block 0177
bad_end: .block 0177
buffer: .block 0200
stage1_bss_end:
