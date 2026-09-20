; monitorfs_pdp10.s -- compact resident MonitorFS primitives.
        .text
        .globl  pdp10_ret_busy
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  mfsdev_io_in
        .globl  mfsdev_io_out
        .globl  mfsdev_storage_errors
        .globl  mfsdev_mtc_words_read
        .globl  mfsdev_mtc_words_written
        .globl  mfsdev_drm_reads
        .globl  mfsdev_drm_writes
        .globl  mfsdev_d6set_reads
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_d6set_blocks_read
        .globl  mfsdev_d6set_blocks_written
        .globl  mfsdev_log_reads
        .globl  mfsdev_log_writes
        .globl  mfsdev_log_blocks_read
        .globl  mfsdev_log_blocks_written
        .globl  mfsdev_log_errors
        .globl  mfsdev_d6set_members
        .globl  proc_swap_blocks_used
        .globl  diskset_runtime_reg_enter

; Full MonitorFS runtime operations.  MINIT freezes detected device names
; into mfsdev_names; a zero slot means that device is absent.
        .globl  mfsdev_names

; Capability masks indexed by device id.  TTY0 has output accounting at the
; logical terminal layer in addition to the physical backend accounting.
; IN:  CTY0 PTR0 CR0 DCS0 GE0 WCNSLS OCNSLS DTC0 MTC0 DSK0 DRM0
; OUT: CTY0 PTP0 CP0 DCS0 GE0 DPY0 TTY0 WCNSLS DTC0 MTC0 DSK0 DRM0 LPT0

; Derive 3/4/6-character device name length from trailing SIXBIT blanks.
; input AC5=name word, output AC6=chars.
mfsdev_name_length:
        movei   6,3
        trnn    5,0777777
        popj    17,
        movei   6,4
        trnn    5,07777
        popj    17,
        movei   6,6
        popj    17,

; Validate AC4 as a present device id.  Return 0/-1 in AC1.
mfsdev_validate_id:
        cail    4,023
        jrst    pdp10_ret_neg1
        skipn   5,mfsdev_names(4)
        jrst    pdp10_ret_neg1
        jrst    pdp10_ret_zero

; int mfsdev_lookup(vnode_t dir, const struct vfs_name *name,
;     vnode_t *nodep)
        .globl  mfsdev_lookup
mfsdev_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        hlrz    4,1
        move    0,4
        subi    0,020000               ; root selector: DEV=0, DEVICES=1
        caile   0,1
        jrst    mfsdev_lookup_dir
        addi    0,2                    ; result kind: endpoint=2, state dir=3
        movei   4,0
mfsdev_lookup_scan:
        cail    4,023
        jrst    pdp10_ret_neg1
        skipn   5,mfsdev_names(4)
        jrst    mfsdev_lookup_next
        came    5,1(2)
        jrst    mfsdev_lookup_next
        pushj   17,mfsdev_name_length
        came    6,(2)
        jrst    mfsdev_lookup_next
        move    5,0
        addi    5,020000               ; provider 2 + selected local kind
        hrl     4,5
        move    5,4
        jrst    mfsdev_lookup_store

mfsdev_lookup_next:
        addi    4,1
        jrst    mfsdev_lookup_scan

mfsdev_lookup_dir:
        caie    4,020003
        jrst    pdp10_ret_neg1
        hrrz    4,1
        pushj   17,mfsdev_validate_id
        jumpn   1,pdp10_ret_neg1
        move    5,(2)
        caie    5,2
        jrst    mfsdev_lookup_stats
        move    5,1(2)
        came    5,[0515700000000]      ; IO
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020002               ; same endpoint vnode as /DEV/name
        jrst    mfsdev_lookup_store
mfsdev_lookup_stats:
        caie    5,5
        jrst    mfsdev_lookup_d6extra
        move    5,1(2)
        came    5,[0636441646300]      ; STATS
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020004
        jrst    mfsdev_lookup_store

mfsdev_lookup_d6extra:
        caie    4,020                  ; D6SET0 only
        jrst    pdp10_ret_neg1
        caie    5,7
        jrst    mfsdev_lookup_swap
        move    5,1(2)
        came    5,[0554555424562]      ; MEMBER
        jrst    pdp10_ret_neg1
        move    5,2(2)
        came    5,[0630000000000]      ; S
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020005
        jrst    mfsdev_lookup_store

mfsdev_lookup_swap:
        caie    5,4
        jrst    mfsdev_lookup_log
        move    5,1(2)
        came    5,[0636741600000]      ; SWAP
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020006
        jrst    mfsdev_lookup_store

mfsdev_lookup_log:
        caie    5,3
        jrst    pdp10_ret_neg1
        move    5,1(2)
        came    5,[0545747000000]      ; LOG
        jrst    pdp10_ret_neg1
        move    5,4
        tlo     5,020007

mfsdev_lookup_store:
        movem   5,(3)
        jrst    pdp10_ret_zero


; Store ent name/type and return 1. AC4=ent, AC5=word, AC6=chars, AC7=type.
mfsdev_readdir_store:
        movem   6,(4)
        movem   5,1(4)
        setzm   2(4)
mfsdev_readdir_store_tail:
        setzm   3(4)
        setzm   4(4)
        movem   7,5(4)
        jrst    pdp10_ret_one

; Set AC7 to the VFS dirent type for an IO endpoint.  AC0=device id.
mfsdev_io_type:
        caige   0,014
        jrst    mfsdev_io_type_char
        caile   0,016
        jrst    mfsdev_io_type_maybe_mount
        movei   7,4                    ; block
        popj    17,
mfsdev_io_type_maybe_mount:
        cain    0,021                  ; DRM0
        jrst    mfsdev_io_type_block
        caie    0,020
        jrst    mfsdev_io_type_char
        movei   7,5                    ; mount source
        popj    17,
mfsdev_io_type_block:
        movei   7,4                    ; block
        popj    17,
mfsdev_io_type_char:
        movei   7,3                    ; char
        popj    17,

; int mfsdev_readdir(vnode_t dir, unsigned int off,
;     struct vfs_dirent *ent)
        .globl  mfsdev_readdir
mfsdev_readdir:
        jumpe   3,pdp10_ret_neg1
        move    4,3
        hlrz    5,1
        move    0,5
        subi    0,020000
        caile   0,1
        jrst    mfsdev_readdir_dir
        xori    0,1                    ; DEV -> 1, DEVICES -> 0
        addi    0,1                    ; DEV=2 (I/O type), DEVICES=1 (dir)
        movei   5,0
        movei   7,0
mfsdev_readdir_scan:
        cail    5,023
        jrst    pdp10_ret_zero
        skipn   6,mfsdev_names(5)
        jrst    mfsdev_readdir_next
        camn    7,2
        jrst    mfsdev_readdir_found
        addi    7,1
mfsdev_readdir_next:
        addi    5,1
        jrst    mfsdev_readdir_scan

mfsdev_readdir_found:
        move    1,5                    ; preserve device id across name length
        move    5,6
        pushj   17,mfsdev_name_length
        caie    0,2
        jrst    mfsdev_readdir_found_dir
        move    0,1
        pushj   17,mfsdev_io_type
        move    4,3
        jrst    mfsdev_readdir_store
mfsdev_readdir_found_dir:
        movei   7,1                    ; directory
        jrst    mfsdev_readdir_store

mfsdev_readdir_dir:
        caie    5,020003
        jrst    pdp10_ret_neg1
        hrrz    0,1
        move    4,0
        pushj   17,mfsdev_validate_id
        jumpn   1,pdp10_ret_neg1
        move    4,3
        jumpe   2,mfsdev_readdir_io
        caie    2,1
        jrst    mfsdev_readdir_extra
mfsdev_readdir_stats:
        move    5,[0636441646300]      ; STATS
        movei   6,5
        movei   7,2                    ; regular
        jrst    mfsdev_readdir_store
mfsdev_readdir_io:
        pushj   17,mfsdev_io_type
        move    4,3
        move    5,[0515700000000]      ; IO
        movei   6,2
        jrst    mfsdev_readdir_store
mfsdev_readdir_extra:
        caie    0,020                  ; extras exist only on D6SET0
        jrst    pdp10_ret_zero
        caie    2,2
        jrst    mfsdev_readdir_swap
        movei   6,7
        move    5,[0554555424562]      ; MEMBER
        movem   6,(4)
        movem   5,1(4)
        move    5,[0630000000000]      ; S
        movem   5,2(4)
        movei   7,2
        jrst    mfsdev_readdir_store_tail
mfsdev_readdir_swap:
        caie    2,3
        jrst    mfsdev_readdir_log
        move    5,[0636741600000]      ; SWAP
        movei   6,4
        movei   7,2
        jrst    mfsdev_readdir_store
mfsdev_readdir_log:
        caie    2,4
        jrst    pdp10_ret_zero
        move    5,[0545747000000]      ; LOG
        movei   6,3
        movei   7,2
        jrst    mfsdev_readdir_store


; int mfsdev_stat(vnode_t node, struct vfs_stat *st)
        .globl  mfsdev_stat
mfsdev_stat:
        jumpe   2,pdp10_ret_neg1
        hlrz    3,1
        hrrz    4,1
        cain    3,020001
        jrst    mfsdev_stat_root
        caie    3,020000
        jrst    mfsdev_stat_not_root
mfsdev_stat_root:
        movei   5,1
        movei   6,0555
        jrst    mfsdev_stat_store

mfsdev_stat_not_root:
        pushj   17,mfsdev_validate_id
        jumpn   1,pdp10_ret_neg1
        cain    3,020003
        jrst    mfsdev_stat_dir
        caie    3,020002
        jrst    mfsdev_stat_file
        caige   4,014
        jrst    mfsdev_stat_char
        caile   4,016
        jrst    mfsdev_stat_maybe_mount
        movei   5,4
        jrst    mfsdev_stat_device_mode
mfsdev_stat_maybe_mount:
        cain    4,021                  ; DRM0
        jrst    mfsdev_stat_block
        caie    4,020
        jrst    mfsdev_stat_char
        movei   5,5
        jrst    mfsdev_stat_device_mode
mfsdev_stat_block:
        movei   5,4
        jrst    mfsdev_stat_device_mode
mfsdev_stat_char:
        movei   5,3
mfsdev_stat_device_mode:
        movei   6,0600
        jrst    mfsdev_stat_store

mfsdev_stat_file:
        caige   3,020004
        jrst    pdp10_ret_neg1
        caile   3,020007
        jrst    pdp10_ret_neg1
        cain    3,020004
        jrst    mfsdev_stat_file_ok
        caie    4,020                  ; extra files exist only on D6SET0
        jrst    pdp10_ret_neg1
mfsdev_stat_file_ok:
        movei   5,2
        movei   6,0444
        jrst    mfsdev_stat_store

mfsdev_stat_dir:
        movei   5,1
        movei   6,0555

mfsdev_stat_store:
        movem   5,(2)
        movem   6,1(2)
        setzm   2(2)
        setzm   3(2)
        jrst    pdp10_ret_zero


; int mfsdev_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  mfsdev_readchar
mfsdev_readchar:
        jumpe   3,pdp10_ret_neg1
        hlrz    6,1                    ; validate_id clobbers AC5
        hrrz    4,1
        pushj   17,mfsdev_validate_id
        jumpn   1,pdp10_ret_neg1
        cain    6,020002
        jrst    pdp10_ret_busy          ; VFS_DEVICE_IO

mfsdev_readchar_not_io:
        cain    6,020004
        jrst    mfsdev_stats_device
        caie    4,020                  ; remaining files are D6SET0 only
        jrst    pdp10_ret_neg1
        cain    6,020005
        jrst    mfsdev_members_readchar
        cain    6,020006
        jrst    mfsdev_stats_swap
        caie    6,020007
        jrst    pdp10_ret_neg1
        movei   5,mfsdev_log_reads
        jrst    mfsdev_stats_readchar

mfsdev_stats_swap:
        ; SWAP reports live allocation state, not lifetime I/O accounting.
        ; Lines are TOTAL, USED, FREE blocks.
        setz    6,
mfsdev_swap_line_loop:
        caig    2,015
        jrst    mfsdev_swap_select
        subi    2,016
        aoja    6,mfsdev_swap_line_loop
mfsdev_swap_select:
        cail    6,3
        jrst    pdp10_ret_zero
        movei   5,6                   ; DISKSET_MRES_OP_SWAP_BLOCKS
        pushj   17,diskset_runtime_reg_enter
        jumpe   6,mfsdev_stats_emit
        cain    6,1
        jrst    mfsdev_swap_used
        sub     1,proc_swap_blocks_used
        jrst    mfsdev_stats_emit
mfsdev_swap_used:
        move    1,proc_swap_blocks_used
        jrst    mfsdev_stats_emit

mfsdev_stats_device:
        setz    5,                     ; zero means ordinary device stats

; Fixed-width bare octal values keep readchar offset handling compact.
; AC4=device id, AC5=0 for device stats or base of five subsystem counters.
mfsdev_stats_readchar:
        setz    6,                     ; line number
mfsdev_stats_line_loop:
        caig    2,015                  ; select once offset is inside a line
        jrst    mfsdev_stats_select
        subi    2,016
        aoja    6,mfsdev_stats_line_loop

mfsdev_stats_select:
        jumpe   5,mfsdev_stats_select_device
        cail    6,5
        jrst    pdp10_ret_zero
        add     5,6
        move    1,(5)
        jrst    mfsdev_stats_emit

mfsdev_stats_select_device:
        cain    6,0
        jrst    mfsdev_stats_device_reads
        cain    6,1
        jrst    mfsdev_stats_device_writes
        caige   4,014
        jrst    mfsdev_stats_device_simple_error
        caile   4,016
        jrst    mfsdev_stats_device_maybe_d6
        move    0,4
        subi    0,014
        jrst    mfsdev_stats_device_native
mfsdev_stats_device_maybe_d6:
        cain    4,022                  ; LPT is a simple output stream
        jrst    mfsdev_stats_device_simple_error
        caile   4,017                  ; 020 D6SET, 021 DRM
        jrst    mfsdev_stats_device_extended_native
        jrst    mfsdev_stats_device_simple_error
mfsdev_stats_device_extended_native:
        move    0,4
        subi    0,015                  ; 020 -> 3, 021 -> 4
        jrst    mfsdev_stats_device_native
mfsdev_stats_device_native:
        caie    6,4
        jrst    mfsdev_stats_device_native_value
        move    1,mfsdev_storage_errors-014(4)
        jrst    mfsdev_stats_emit
mfsdev_stats_device_native_value:
        caige   6,2
        jrst    pdp10_ret_zero
        caile   6,3
        jrst    pdp10_ret_zero
        move    5,6
        subi    5,2                    ; read/write selector
        cain    0,1                    ; MTC has separate word volume
        jrst    mfsdev_stats_native_mtc
        cain    0,3                    ; D6SET has aggregate block volume
        jrst    mfsdev_stats_native_d6
        ; DTC, DSK and DRM are one native unit per request.
        jumpe   5,mfsdev_stats_device_reads
        jrst    mfsdev_stats_device_writes
mfsdev_stats_native_mtc:
        move    1,mfsdev_mtc_words_read(5)
        jrst    mfsdev_stats_emit
mfsdev_stats_native_d6:
        move    1,mfsdev_d6set_blocks_read(5)
        jrst    mfsdev_stats_emit
mfsdev_stats_device_simple_error:
        caie    6,2
        jrst    pdp10_ret_zero
        setz    1,
        cain    4,022                  ; LPT has no storage-error counter
        jrst    mfsdev_stats_emit
        caige   4,014
        jrst    mfsdev_stats_emit
        move    1,mfsdev_storage_errors-014(4)
        jrst    mfsdev_stats_emit

mfsdev_stats_device_reads:
        movei   0,1
        lsh     0,0(4)
        tdnn    0,[076325]
        jrst    mfsdev_stats_zero
        move    1,mfsdev_io_in(4)
        jrst    mfsdev_stats_emit
mfsdev_stats_device_writes:
        movei   0,1
        lsh     0,0(4)
        tdnn    0,[01073751]
        jrst    mfsdev_stats_zero
        move    1,mfsdev_io_out(4)
        jrst    mfsdev_stats_emit
mfsdev_stats_zero:
        setz    1,

; Emit one fixed-width bare value: 12 octal digits CR LF = 016 chars.
; Native octal formatting avoids a decimal formatter in resident KCORE.
mfsdev_stats_emit:
        caile   2,013                  ; columns 0..11 are octal digits
        jrst    mfsdev_stats_eol
        move    6,2
        imuli   6,-3
        addi    6,041                  ; bit shift 33..0
        move    0,1
        lsh     0,0(6)
        andi    0,7
        addi    0,060
        jrst    mfsdev_stats_store
mfsdev_stats_eol:
        movei   0,015                  ; normalized offset is either CR or LF
        caie    2,014
        movei   0,012
mfsdev_stats_store:
        movem   0,(3)
        jrst    pdp10_ret_one

; D6SET MEMBERS is one fixed four-character line per configured member:
; two octal unit digits plus CR LF.  D6SET currently contains only DSK units,
; so repeating the DSK prefix in every line would waste resident formatter code.
mfsdev_members_readchar:
        move    6,2
        lsh     6,-2                   ; member index = off / 4
        move    5,mfsdev_d6set_members
        move    0,5
        andi    0,7                    ; member count
        caml    6,0
        jrst    pdp10_ret_zero
        andi    2,3                    ; column = off % 4
        jumpe   2,mfsdev_members_zero
        caie    2,1
        jrst    mfsdev_members_eol
        imuli   6,-3
        subi    6,3
        lsh     5,0(6)
        andi    5,7
        movei   0,060(5)
        jrst    mfsdev_members_store
mfsdev_members_zero:
        movei   0,060
        jrst    mfsdev_members_store
mfsdev_members_eol:
        movei   0,015
        caie    2,2
        movei   0,012
mfsdev_members_store:
        jrst    mfsdev_stats_store


; Process view -- compact dynamic MonitorFS process state.
;
; The process table has three words per slot.  Keep synthetic namespace and
; fixed-format file reads in PDP-6 code so descriptor operations do not pay
; GCC save-frame costs.

        .equ    PROC_WORDS,3
        .equ    PROC_STATE_LH_MASK,0700000

        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot

; AC1 = slot. Return AC1 = active struct proc address, or zero.
; AC2..AC4 are caller-scratch.
mfsproc_proc_ptr:
        skipn   2,proc_table
        jrst    pdp10_ret_zero
        caml    1,proc_slots
        jrst    pdp10_ret_zero
        move    3,1
        lsh     3,1
        add     3,1
        add     3,2
        hlrz    4,2(3)
        andi    4,PROC_STATE_LH_MASK
        jumpe   4,pdp10_ret_zero
        move    1,3
        popj    17,

; AC1 = struct vfs_name *. Return AC1 = decimal value 0..255, or -1.
; AC3 and AC7 are preserved; AC2 and AC4..AC6 are caller-scratch.
; Names longer than three digits are rejected before the packed word is read.
        .globl  mfsproc_parse_slot
mfsproc_parse_slot:
        jumpe   1,pdp10_ret_neg1
        move    2,(1)
        jumpe   2,pdp10_ret_neg1
        cail    2,4
        jrst    pdp10_ret_neg1
        move    4,1(1)
        setz    5,
mfsproc_parse_slot_loop:
        move    6,4
        lsh     6,-036
        andi    6,077
        cail    6,020
        cail    6,032
        jrst    pdp10_ret_neg1
        subi    6,020
        imuli   5,012
        add     5,6
        lsh     4,6
        sojg    2,mfsproc_parse_slot_loop
        cail    5,0400
        jrst    pdp10_ret_neg1
        move    1,5
        popj    17,

; AC1 = slot (0..255). Return AC1 = chars, AC2 = SIXBIT decimal name.
; AC3 is preserved; AC4..AC7 are caller-scratch.
        .globl  mfsproc_format_slot
mfsproc_format_slot:
        move    4,1
        idivi   4,0144                  ; hundreds, remainder
        move    6,5
        idivi   6,012                   ; tens, ones
        move    2,4
        lsh     2,6
        ior     2,6
        lsh     2,6
        ior     2,7
        addi    2,0202020               ; convert all three digits to SIXBIT
        lsh     2,022                   ; left-justify three characters
        movei   1,3
        jumpn   4,mfsproc_format_done
        lsh     2,6                     ; discard leading zero
        subi    1,1
        jumpn   6,mfsproc_format_done
        lsh     2,6
        subi    1,1
mfsproc_format_done:
        popj    17,

        .globl  mfsdom_exists

; MonitorFS process view and MonitorFS domain view share provider 3.  MonitorFS domain view nodes carry bit 0400000 in
; the vnode RH; fold their namespace operations into these leaves so both
; namespaces share decimal parsing, dirent stores and stat stores.
;
; int mfsproc_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
        .globl  mfsproc_lookup
mfsproc_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        move    0,1
        andi    0,0400000
        hlrz    4,1
        caie    4,030001
        jrst    mfsproc_lookup_proc
        move    7,3
        move    1,2
        pushj   17,mfsproc_parse_slot
        camn    1,[-1]
        jrst    pdp10_ret_neg1
        move    6,1
        jumpn   0,mfsproc_lookup_domain_root
        jrst    mfsproc_lookup_slot
mfsproc_lookup_slot:
        move    1,6
        pushj   17,mfsproc_proc_ptr
        jumpe   1,pdp10_ret_neg1
        move    1,6
        tlo     1,030002
        jrst    mfsproc_lookup_store
mfsproc_lookup_domain_root:
        move    1,6
        pushj   17,mfsdom_exists
        jumpe   1,pdp10_ret_neg1
        move    1,6
        tlo     1,030002
        tro     1,0400000
        jrst    mfsproc_lookup_store
mfsproc_lookup_proc:
        caie    4,030002
        jrst    pdp10_ret_neg1
        jumpn   0,mfsproc_lookup_domain_proc
        move    5,2
        move    7,3
        hrrz    6,1
        move    1,6
        pushj   17,mfsproc_proc_ptr
        jumpe   1,pdp10_ret_neg1
        move    2,(5)
        move    3,1(5)
        movei   4,0
        caie    2,4
        jrst    mfsproc_lookup_len5
        camn    3,mfsproc_names+0
        movei   4,3
        camn    3,mfsproc_names+3
        movei   4,6
        jrst    mfsproc_lookup_have_kind
mfsproc_lookup_len5:
        caie    2,5
        jrst    mfsproc_lookup_len6
        camn    3,mfsproc_names+1
        movei   4,4
        camn    3,mfsproc_names+2
        movei   4,5
        jrst    mfsproc_lookup_have_kind
mfsproc_lookup_len6:
        caie    2,6
        jrst    pdp10_ret_neg1
        camn    3,mfsproc_names+4
        movei   4,7
mfsproc_lookup_have_kind:
        jumpe   4,pdp10_ret_neg1
        move    1,6
        hrl     1,4
        tlo     1,030000
        jrst    mfsproc_lookup_store
mfsproc_lookup_domain_proc:
        move    5,2
        move    7,3
        move    6,1
        andi    6,0377
        move    2,(5)
        caie    2,6
        jrst    pdp10_ret_neg1
        move    3,1(5)
        came    3,[0636441646563]      ; STATUS
        jrst    pdp10_ret_neg1
        move    1,6
        pushj   17,mfsdom_exists
        jumpe   1,pdp10_ret_neg1
        move    1,6
        tlo     1,030007
        tro     1,0400000
mfsproc_lookup_store:
        movem   1,(7)
        jrst    pdp10_ret_zero

; int mfsproc_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
        .globl  mfsproc_readdir
mfsproc_readdir:
        jumpe   3,pdp10_ret_neg1
        move    0,1
        andi    0,0400000
        hlrz    4,1
        caie    4,030001
        jrst    mfsproc_readdir_proc
        jumpn   0,mfsproc_readdir_domain_root
        skipn   5,proc_table
        jrst    pdp10_ret_zero
        movei   6,0
        movei   7,0
        jrst    mfsproc_readdir_root_loop
mfsproc_readdir_root_loop:
        caml    6,proc_high_slot
        jrst    pdp10_ret_zero
        hlrz    4,2(5)
        andi    4,PROC_STATE_LH_MASK
        jumpe   4,mfsproc_readdir_root_next
        camn    7,2
        jrst    mfsproc_readdir_root_found
        addi    7,1
mfsproc_readdir_root_next:
        addi    5,PROC_WORDS
        addi    6,1
        jrst    mfsproc_readdir_root_loop
mfsproc_readdir_domain_root:
        move    7,2
        movei   6,0
mfsproc_readdir_domain_loop:
        move    1,6
        pushj   17,mfsdom_exists
        jumpe   1,mfsproc_readdir_domain_next
        jumpe   7,mfsproc_readdir_root_found
        subi    7,1
mfsproc_readdir_domain_next:
        addi    6,1
        cail    6,0400
        jrst    pdp10_ret_zero
        jrst    mfsproc_readdir_domain_loop
mfsproc_readdir_root_found:
        move    1,6
        pushj   17,mfsproc_format_slot
        move    7,3
        move    5,2
        move    4,1
        movei   6,1
        jrst    mfsproc_readdir_store
mfsproc_readdir_proc:
        caie    4,030002
        jrst    pdp10_ret_neg1
        jumpn   0,mfsproc_readdir_domain_proc
        move    7,3
        move    6,2
        hrrz    1,1
        pushj   17,mfsproc_proc_ptr
        jumpe   1,pdp10_ret_neg1
        cail    6,5
        jrst    pdp10_ret_zero
        move    5,mfsproc_names(6)
        movei   4,5
        caie    6,0
        cain    6,3
        movei   4,4
        cain    6,4
        movei   4,6
mfsproc_readdir_proc_store:
        movei   6,2
        jrst    mfsproc_readdir_store
mfsproc_readdir_domain_proc:
        move    7,2                    ; preserve off across exists
        andi    1,0377
        pushj   17,mfsdom_exists
        jumpe   1,pdp10_ret_neg1
        jumpn   7,pdp10_ret_zero
        move    7,3
        movei   4,6
        move    5,[0636441646563]      ; STATUS
        movei   6,2
mfsproc_readdir_store:
        movem   4,(7)
        movem   5,1(7)
        setzm   2(7)
        setzm   3(7)
        setzm   4(7)
        movem   6,5(7)
        jrst    pdp10_ret_one

; int mfsproc_stat(vnode_t node, struct vfs_stat *st)
        .globl  mfsproc_stat
mfsproc_stat:
        jumpe   2,pdp10_ret_neg1
        move    7,2
        move    0,1
        andi    0,0400000
        hlrz    5,1
        cain    5,030000               ; MonitorFS root
        jrst    mfsproc_stat_dir
        caie    5,030001
        jrst    mfsproc_stat_nonroot
mfsproc_stat_dir:
        movei   3,1
        movei   4,0555
        jrst    mfsproc_stat_store_zero
mfsproc_stat_nonroot:
        jumpn   0,mfsproc_stat_domain
        hrrz    6,1
        move    1,6
        pushj   17,mfsproc_proc_ptr
        jumpe   1,pdp10_ret_neg1
        cain    5,030002
        jrst    mfsproc_stat_dir
mfsproc_stat_file:
        caige   5,030003
        jrst    pdp10_ret_neg1
        caile   5,030007
        jrst    pdp10_ret_neg1
        movei   3,2
        movei   4,0444
        jrst    mfsproc_stat_store_zero
mfsproc_stat_domain:
        move    6,1
        andi    6,0377
        caie    5,030002
        jrst    mfsproc_stat_domain_status
        move    1,6
        pushj   17,mfsdom_exists
        jumpe   1,pdp10_ret_neg1
        jrst    mfsproc_stat_dir
mfsproc_stat_domain_status:
        caie    5,030007
        jrst    pdp10_ret_neg1
        move    1,6
        pushj   17,mfsdom_exists
        jumpe   1,pdp10_ret_neg1
        movei   3,2
        movei   4,0444
        movei   5,6
        jrst    mfsproc_stat_store_words
mfsproc_stat_store_zero:
        setz    5,
mfsproc_stat_store_words:
        movem   3,(7)
        movem   4,1(7)
        setzm   2(7)
        movem   5,3(7)
        jrst    pdp10_ret_zero


; Return printable process-state SIXBIT word and length.
; AC1 = struct proc *, AC2 = slot. Return AC1 = word, AC2 = chars.
mfsproc_state_word:
        hlrz    3,2(1)
        andi    3,PROC_STATE_LH_MASK
        lsh     3,-017
        cain    3,4                    ; ZOMB outranks nonresident/SWAP
        jrst    mfsproc_state_not_swapped
        jumpe   2,mfsproc_state_not_swapped
        hrrz    4,1(1)
        jumpe   4,mfsproc_state_swapped
mfsproc_state_not_swapped:
        move    1,mfsproc_state_names(3)
        movei   2,4
        caig    3,2                    ; active IDLE/RUN names have 3 chars
        movei   2,3
        cain    3,3                    ; SLEEP
        movei   2,5
        popj    17,
mfsproc_state_swapped:
        move    1,[0636741600000]
        movei   2,4
        popj    17,

; STATUS reader. AC1 = proc *, AC2 = slot, AC5 = off, AC7 = chp.
mfsproc_status_readchar:
        cail    5,024
        jrst    mfsproc_status_tail
        move    3,5
        lsh     3,-2
        move    4,5
        andi    4,3
        cain    4,3
        jrst    mfsproc_status_space
        jumpe   3,mfsproc_status_pid
        cain    3,1
        jrst    mfsproc_status_ppid
        cain    3,2
        jrst    mfsproc_status_pgrp
        pushj   17,proc_scope_id
        move    6,1
        caie    3,3
        lsh     6,-010
        andi    6,0377
        jrst    mfsproc_status_digit
mfsproc_status_pid:
        move    6,2
        jrst    mfsproc_status_digit
mfsproc_status_ppid:
        ldb     6,[POINT 8,(1),27]
        jrst    mfsproc_status_digit
mfsproc_status_pgrp:
        hrrz    6,(1)
        andi    6,0377
mfsproc_status_digit:
        jumpe   4,mfsproc_status_digit0
        caie    4,1
        jrst    mfsproc_status_digit2
        lsh     6,-3
        jrst    mfsproc_status_digit_store
mfsproc_status_digit0:
        lsh     6,-6
mfsproc_status_digit2:
mfsproc_status_digit_store:
        andi    6,7
        addi    6,060
mfsproc_status_store_one:
        movem   6,(7)
        jrst    pdp10_ret_one
mfsproc_status_space:
        movei   6,040
        jrst    mfsproc_status_store_one
mfsproc_status_tail:
        caie    5,024
        jrst    mfsproc_status_cr
        pushj   17,mfsproc_state_word
        move    6,1
        lsh     6,-036
        andi    6,077
        addi    6,040
        jrst    mfsproc_status_store_one
mfsproc_status_cr:
        caie    5,025
        jrst    mfsproc_status_lf
        movei   6,015
        jrst    mfsproc_status_store_one
mfsproc_status_lf:
        caie    5,026
        jrst    pdp10_ret_zero
        movei   6,012
        jrst    mfsproc_status_store_one

; int mfsproc_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  mfsproc_readchar
        .globl  vfs_sixbit_readchar
        .globl  kfmt_u18_decimal_readchar
        .globl  proc_scope_id
        .globl  proc_comm_words
mfsproc_readchar:
        trne    1,0400000
        jrst    pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        move    7,3
        move    5,2
        move    6,1
        hlrz    4,1
        caige   4,030003
        jrst    pdp10_ret_neg1
        caile   4,030007
        jrst    pdp10_ret_neg1
        hrrz    1,6
        pushj   17,mfsproc_proc_ptr
        jumpe   1,pdp10_ret_neg1
        hlrz    4,6
        hrrz    2,6
        caie    4,030006
        jrst    mfsproc_readchar_not_comm
        movei   4,2
        jumpe   2,mfsproc_readchar_comm0
        cain    2,1
        movei   4,1
        jrst    mfsproc_readchar_comm_load
mfsproc_readchar_comm0:
        movei   4,0
mfsproc_readchar_comm_load:
        move    1,proc_comm_words(4)
        movei   2,6
        move    3,5
        move    4,7
        jrst    vfs_sixbit_readchar
mfsproc_readchar_not_comm:
        cain    4,030007
        jrst    mfsproc_status_readchar
mfsproc_readchar_not_status:
        caie    4,030004
        jrst    mfsproc_readchar_numeric
        pushj   17,mfsproc_state_word
        move    3,5
        move    4,7
        jrst    vfs_sixbit_readchar
mfsproc_readchar_numeric:
        caie    4,030003
        jrst    mfsproc_readchar_words
        ldb     1,[POINT 8,(1),27]
        jrst    mfsproc_readchar_number
mfsproc_readchar_words:
        caie    4,030005
        jrst    pdp10_ret_neg1
        hlrz    1,1(1)
mfsproc_readchar_number:
        move    2,5
        move    3,7
        jrst    kfmt_u18_decimal_readchar

        .data
mfsproc_names:
        .word   0606051440000
        .word   0636441644500
        .word   0675762446300
        .word   0435755550000
        .word   0636441646563
mfsproc_state_names:
        .word   0466245450000
        .word   0514454000000
        .word   0626556000000
        .word   0635445456000
        .word   0725755420000
        .word   0466245450000
        .word   0636457600000


; Domain view -- compact MonitorFS domain-state leaves.
;
; The portable MonitorFS implementation remains the host/reference implementation.  This
; Domain leaves share the compact process-view conventions so synthetic
; process-domain directory operations do not pay GCC frame and unsigned-
; arithmetic costs in permanent KCORE.
;
; Domain IDs are 0..255.  Live process scope is stored in the stable u-area
; control word; zombie scope is retained in the scheduler word.  proc_scope_id
; already normalizes those representations, so scans use it rather than
; duplicating the scope lifetime rules here.

        .equ    PROC_STATE_ZOMB_LH,0400000
        .equ    PROC_STATE_STOP_LH,0600000
        .equ    PROC_MAX_SLOTS,0400
        .equ    DOMAIN_TAG,0400000
        .equ    DOMAIN_MASK,0377
        .equ    DOMAIN_SHIFT,010
        .equ    DOMAIN_STATUS_WORDS,6

        .text
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_one
        .globl  pdp10_ret_neg1
        .globl  proc_table
        .globl  proc_high_slot
        .globl  proc_swap_records
        .globl  proc_scope_id
        .globl  mfsproc_parse_slot
        .globl  mfsproc_format_slot

; AC1 = domain ID. Return AC1 = 1 if at least one active process belongs to
; the domain, otherwise zero.  Reuse the STATUS scanner with no output buffer.
        .globl  mfsdom_exists
mfsdom_exists:
        ; Predicate callers retain their namespace state in AC3/AC6/AC7.
        ; The shared scanner uses all three, so preserve them here.
        push    17,3
        push    17,6
        push    17,7
        move    5,1                    ; requested domain
        setz    7,                     ; no status buffer => existence only
        pushj   17,mfsdom_scan
        pop     17,7
        pop     17,6
        pop     17,3
        popj    17,

; Fill six DOMAIN/ID/STATUS words at AC2 for domain AC1.  The same scan serves
; existence tests so process-table/domain matching is not duplicated.
mfsdom_status:
        move    7,2
        move    5,1
        movem   1,(7)
        setzm   1(7)
        setzm   2(7)
        setzm   3(7)
        setzm   4(7)
        setzm   5(7)
mfsdom_scan:
        skipn   6,proc_table
        jrst    mfsdom_scan_done
        movei   4,0
mfsdom_scan_loop:
        caml    4,proc_high_slot
        jrst    mfsdom_scan_done
        hlrz    3,2(6)
        andi    3,PROC_STATE_LH_MASK
        jumpe   3,mfsdom_scan_next
        move    1,6
        pushj   17,proc_scope_id
        lsh     1,-DOMAIN_SHIFT
        andi    1,DOMAIN_MASK
        came    1,5
        jrst    mfsdom_scan_next
        jumpe   7,pdp10_ret_one
        aos     1(7)
        cain    3,PROC_STATE_ZOMB_LH
        jrst    mfsdom_scan_next
        hrrz    1,1(6)
        jumpe   1,mfsdom_scan_nonresident
        hlrz    1,1(6)
        addm    1,2(7)
        jrst    mfsdom_scan_stop
mfsdom_scan_nonresident:
        jumpe   4,mfsdom_scan_stop
        skipn   1,proc_swap_records
        jrst    mfsdom_scan_stop
        add     1,4
        skipn   2,(1)
        jrst    mfsdom_scan_stop
        aos     3(7)
        hrrz    2,2
        addm    2,4(7)
mfsdom_scan_stop:
        cain    3,PROC_STATE_STOP_LH
        aos     5(7)
mfsdom_scan_next:
        addi    6,PROC_WORDS
        aoja    4,mfsdom_scan_loop
mfsdom_scan_done:
        jumpe   7,pdp10_ret_zero
        move    1,1(7)
        popj    17,

; int mfsdom_read_words(vnode_t node, unsigned int off, kword_t *buf,
;     unsigned int nwords)
        .globl  mfsdom_read_words
mfsdom_read_words:
        jumpe   3,pdp10_ret_neg1
        hlrz    5,1
        caie    5,030007
        jrst    pdp10_ret_neg1
        move    5,1
        andi    5,DOMAIN_MASK
        cail    2,DOMAIN_STATUS_WORDS
        jrst    pdp10_ret_zero
        ; Six status words plus saved off/buf/nwords.  This is transient stack
        ; storage only; no resident process or KCORE data is added.
        add     17,[011,,011]
        movem   2,-2(17)
        movem   3,-1(17)
        movem   4,(17)
        move    1,5
        movei   2,-010(17)
        pushj   17,mfsdom_status
        jumpe   1,mfsdom_read_missing
        move    5,-2(17)
        move    6,-1(17)
        move    7,(17)
        movei   1,0
mfsdom_read_copy:
        cail    5,DOMAIN_STATUS_WORDS
        jrst    mfsdom_read_done
        jumpe   7,mfsdom_read_done
        movei   2,-010(17)
        add     2,5
        move    3,(2)
        movem   3,(6)
        addi    6,1
        addi    5,1
        subi    7,1
        aoja    1,mfsdom_read_copy
mfsdom_read_done:
        sub     17,[011,,011]
        popj    17,
mfsdom_read_missing:
        sub     17,[011,,011]
        jrst    pdp10_ret_neg1
