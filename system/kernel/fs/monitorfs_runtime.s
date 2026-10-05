; monitorfs_runtime.s -- compact resident MonitorFS primitives.
        .text
        .globl  kret_busy
        .globl  kret_zero
        .globl  kret_neg1
        .globl  mfsdev_io_in
        .globl  mfsdev_io_out
        .globl  mfsdev_storage_errors
        .globl  mfsdev_drm_reads
        .globl  mfsdev_drm_writes
        .globl  mfsdev_d6set_reads
        .globl  mfsdev_d6set_writes
        .globl  mfsdev_d6set_members
        .globl  proc_swap_blocks_used
        .globl  blockset_tail_blocks

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
        cail    4,024
        jrst    kret_neg1
        skipn   5,mfsdev_names(4)
        jrst    kret_neg1
        jrst    kret_zero

; int mfsdev_lookup(vnode_t dir, const struct vfs_name *name,
;     vnode_t *nodep)
        .globl  mfsdev_lookup
mfsdev_lookup:
        jumpe   2,kret_neg1
        jumpe   3,kret_neg1
        hlrz    4,1
        move    0,4
        subi    0,020000               ; root selector: DEV=0, DEVICES=1
        caile   0,1
        jrst    mfsdev_lookup_dir
        addi    0,2                    ; result kind: endpoint=2, state dir=3
        movei   4,0
mfsdev_lookup_scan:
        cail    4,024
        jrst    kret_neg1
        skipn   5,mfsdev_names(4)
        jrst    mfsdev_lookup_next
        came    5,1(2)                 ; first SIXBIT name word is universal
        jrst    mfsdev_lookup_next
        caie    4,023                  ; TTYDPY0 alone has a seventh character
        jrst    mfsdev_lookup_short
        movei   6,7
        came    6,(2)
        jrst    mfsdev_lookup_next
        move    6,2(2)
        came    6,[200000000000]       ; SIXBIT /0     /
        jrst    mfsdev_lookup_next
        jrst    mfsdev_lookup_match
mfsdev_lookup_short:
        pushj   17,mfsdev_name_length
        came    6,(2)
        jrst    mfsdev_lookup_next
mfsdev_lookup_match:
        move    5,0
        addi    5,020000               ; provider 2 + selected local kind
        hrl     4,5
        move    5,4
        jrst    mfsdev_lookup_store

mfsdev_lookup_next:
        aoja    4,mfsdev_lookup_scan

mfsdev_lookup_dir:
        caie    4,020003
        jrst    kret_neg1
        move    7,3                    ; mfsleaf_lookup uses AC3 scratch
        hrrz    6,1
        move    4,6
        pushj   17,mfsdev_validate_id
        jumpn   1,kret_neg1
        move    1,2
        movei   2,mfsdev_leaf_names
        movei   0,2                    ; ordinary device: IO, STATS
        caie    6,020                  ; D6SET0 has three extra leaves
        jrst    mfsdev_lookup_leaf
        movei   0,4                    ; IO, STATS, MEMBERS, SWAP
mfsdev_lookup_leaf:
        pushj   17,mfsleaf_lookup
        jumpl   1,kret_neg1
        move    4,1                    ; leaf index
        jumpe   4,mfsdev_lookup_leaf_io
        addi    4,3                    ; STATS=4, MEMBERS=5, SWAP=6
        jrst    mfsdev_lookup_leaf_kind
mfsdev_lookup_leaf_io:
        movei   4,2
mfsdev_lookup_leaf_kind:
        movei   5,020000
        add     5,4
        hrl     6,5
        move    5,6
        move    3,7

mfsdev_lookup_store:
        movem   5,(3)
        jrst    kret_zero


; Store ent name/type and return 1. AC4=ent, AC5=word, AC6=chars, AC7=type.
mfsdev_readdir_store:
        movem   6,(4)
        movem   5,1(4)
        setzm   2(4)
        caie    1,023                  ; TTYDPY0 carries its final '0' in word 2
        jrst    mfsdev_readdir_store_tail
        move    5,[200000000000]
        movem   5,2(4)
mfsdev_readdir_store_tail:
        setzm   3(4)
        setzm   4(4)
        movem   7,5(4)
        jrst    kret_one

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
        jumpe   3,kret_neg1
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
        cail    5,024
        jrst    kret_zero
        skipn   6,mfsdev_names(5)
        jrst    mfsdev_readdir_next
        camn    7,2
        jrst    mfsdev_readdir_found
        addi    7,1
mfsdev_readdir_next:
        aoja    5,mfsdev_readdir_scan

mfsdev_readdir_found:
        move    1,5                    ; preserve device id across name length
        move    5,6                    ; first SIXBIT name word
        caie    1,023
        jrst    mfsdev_readdir_found_short
        movei   6,7                    ; TTYDPY0
        jrst    mfsdev_readdir_found_type
mfsdev_readdir_found_short:
        pushj   17,mfsdev_name_length
mfsdev_readdir_found_type:
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
        jrst    kret_neg1
        hrrz    0,1
        move    4,0
        pushj   17,mfsdev_validate_id
        jumpn   1,kret_neg1
        move    6,0                    ; preserve device id
        movei   1,mfsdev_leaf_names
        movei   0,2
        caie    6,020
        jrst    mfsdev_readdir_leaf
        movei   0,4                    ; IO, STATS, MEMBERS, SWAP
mfsdev_readdir_leaf:
        push    17,2                   ; preserve leaf offset for IO type
        pushj   17,mfsleaf_readdir
        pop     17,2
        caie    1,1
        popj    17,
        jumpn   2,mfsdev_readdir_leaf_done
        move    0,6
        pushj   17,mfsdev_io_type
        movem   7,5(3)
mfsdev_readdir_leaf_done:
        movei   1,1
        popj    17,


; int mfsdev_stat(vnode_t node, struct vfs_stat *st)
        .globl  mfsdev_stat
mfsdev_stat:
        jumpe   2,kret_neg1
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
        jumpn   1,kret_neg1
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
        jrst    kret_neg1
        caile   3,020006
        jrst    kret_neg1
        cain    3,020004
        jrst    mfsdev_stat_file_ok
        caie    4,020                  ; extra files exist only on D6SET0
        jrst    kret_neg1
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
        jrst    kret_zero


; int mfsdev_readchar(vnode_t node, kword_t off, unsigned int *chp)
        .globl  mfsdev_readchar
mfsdev_readchar:
        jumpe   3,kret_neg1
        hlrz    6,1                    ; validate_id clobbers AC5
        hrrz    4,1
        pushj   17,mfsdev_validate_id
        jumpn   1,kret_neg1
        cain    6,020002
        jrst    kret_busy          ; VFS_DEVICE_IO

mfsdev_readchar_not_io:
        cain    6,020004
        jrst    mfsdev_stats_device
        caie    4,020                  ; remaining files are D6SET0 only
        jrst    kret_neg1
        cain    6,020005
        jrst    mfsdev_members_readchar
        caie    6,020006
        jrst    kret_neg1
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
        jrst    kret_zero
        pushj   17,blockset_tail_blocks
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
        jrst    kret_zero
        add     5,6
        move    1,(5)
        jrst    mfsdev_stats_emit

mfsdev_stats_select_device:
        jumpe   6,mfsdev_stats_device_read_line
        cain    6,1
        jrst    mfsdev_stats_device_write_line
        caie    6,2                    ; third and final line is errors
        jrst    kret_zero
        setz    1,                     ; non-storage devices report zero
        caige   4,014                  ; DTC0 is first storage-error slot
        jrst    mfsdev_stats_emit
        caile   4,021                  ; DRM0 is last storage-error slot
        jrst    mfsdev_stats_emit
        move    1,mfsdev_storage_errors-014(4)
        jrst    mfsdev_stats_emit

mfsdev_stats_device_read_line:
        cain    4,023                  ; TTYDPY0 input is CTY0
        jrst    mfsdev_stats_ttydpy_reads
        cain    4,020                  ; D6SET is an aggregate mount source
        jrst    mfsdev_stats_d6_reads
        jrst    mfsdev_stats_device_reads
mfsdev_stats_device_write_line:
        cain    4,023                  ; TTYDPY0 output is DPY0
        jrst    mfsdev_stats_ttydpy_writes
        cain    4,020
        jrst    mfsdev_stats_d6_writes
        jrst    mfsdev_stats_device_writes
mfsdev_stats_d6_reads:
        move    1,mfsdev_d6set_reads
        jrst    mfsdev_stats_emit
mfsdev_stats_d6_writes:
        move    1,mfsdev_d6set_writes
        jrst    mfsdev_stats_emit
mfsdev_stats_ttydpy_reads:
        move    1,mfsdev_io_in
        jrst    mfsdev_stats_emit
mfsdev_stats_ttydpy_writes:
        move    1,mfsdev_io_out+010
        jrst    mfsdev_stats_emit

mfsdev_stats_device_reads:
        movei   0,1
        lsh     0,0(4)
        tdnn    0,[0476325]
        jrst    mfsdev_stats_zero
        move    1,mfsdev_io_in(4)
        jrst    mfsdev_stats_emit
mfsdev_stats_device_writes:
        movei   0,1
        lsh     0,0(4)
        tdnn    0,[01473751]
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
        imuli   6,3
        subi    6,041                  ; bit shift -33..0
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
        jrst    kret_one

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
        jrst    kret_zero
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
        .globl  kret_zero
        .globl  kret_one
        .globl  kret_neg1
        .globl  proc_table
        .globl  proc_slots
        .globl  proc_high_slot

; AC1 = slot. Return AC1 = active struct proc address, or zero.
; AC2..AC4 are caller-scratch.
mfsproc_proc_ptr:
        skipn   2,proc_table
        jrst    kret_zero
        caml    1,proc_slots
        jrst    kret_zero
        move    3,1
        lsh     3,1
        add     3,1
        add     3,2
        hlrz    4,2(3)
        andi    4,PROC_STATE_LH_MASK
        jumpe   4,kret_zero
        move    1,3
        popj    17,

; AC1 = struct vfs_name *. Return AC1 = decimal value 0..255, or -1.
; AC3 and AC7 are preserved; AC2 and AC4..AC6 are caller-scratch.
; Names longer than three digits are rejected before the packed word is read.
        .globl  mfsproc_parse_slot
mfsproc_parse_slot:
        jumpe   1,kret_neg1
        move    2,(1)
        jumpe   2,kret_neg1
        cail    2,4
        jrst    kret_neg1
        move    4,1(1)
        setz    5,
mfsproc_parse_slot_loop:
        move    6,4
        lsh     6,-036
        andi    6,077
        cail    6,020
        cail    6,032
        jrst    kret_neg1
        subi    6,020
        imuli   5,012
        add     5,6
        lsh     4,6
        sojg    2,mfsproc_parse_slot_loop
        cail    5,0400
        jrst    kret_neg1
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

; Leaf-name tables use two words per entry.  Every leaf is at most 11
; characters, so the low SIXBIT slot of word 2 is otherwise zero; store the
; character count in those low six bits.  This saves one permanent word per
; leaf without adding a separate length table.
; AC1=name pointer, AC2=table pointer, AC0=entry count.
; Return AC1=leaf index or -1.
mfsleaf_lookup:
        setz    4,
mfsleaf_lookup_loop:
        move    5,(1)                  ; requested character count
        move    3,1(2)
        andi    3,077                  ; packed table character count
        came    5,3
        jrst    mfsleaf_lookup_next
        move    5,1(1)
        came    5,(2)
        jrst    mfsleaf_lookup_next
        move    5,2(1)
        move    3,1(2)
        andcmi  3,077                  ; remove packed character count
        came    5,3
        jrst    mfsleaf_lookup_next
        move    1,4
        popj    17,
mfsleaf_lookup_next:
        addi    2,2
        aoj     4,
        caml    4,0
        jrst    mfsleaf_lookup_missing
        jrst    mfsleaf_lookup_loop
mfsleaf_lookup_missing:
        seto    1,
        popj    17,

; AC1=table pointer, AC2=leaf index, AC3=struct vfs_dirent *, AC0=count.
mfsleaf_readdir:
        caml    2,0
        jrst    kret_zero
        move    4,2
        lsh     4,1
        add     1,4
        move    4,1(1)
        move    5,4
        andi    5,077                  ; character count
        movem   5,(3)
        andcmi  4,077                  ; second packed SIXBIT word
        move    5,(1)
        movem   5,1(3)
        movem   4,2(3)
        setzm   3(3)
        setzm   4(3)
        movei   4,2
        movem   4,5(3)
        jrst    kret_one

; MonitorFS process view and MonitorFS domain view share provider 3.  MonitorFS domain view nodes carry bit 0400000 in
; the vnode RH; fold their namespace operations into these leaves so both
; namespaces share decimal parsing, dirent stores and stat stores.
;
; int mfsproc_lookup(vnode_t dir, const struct vfs_name *name, vnode_t *nodep)
        .globl  mfsproc_lookup
mfsproc_lookup:
        jumpe   2,kret_neg1
        jumpe   3,kret_neg1
        move    0,1
        andi    0,0400000
        hlrz    4,1
        caie    4,030001
        jrst    mfsproc_lookup_proc
        move    7,3
        move    1,2
        pushj   17,mfsproc_parse_slot
        camn    1,[-1]
        jrst    kret_neg1
        move    6,1
        jumpn   0,mfsproc_lookup_domain_root
        jrst    mfsproc_lookup_slot
mfsproc_lookup_slot:
        move    1,6
        pushj   17,mfsproc_proc_ptr
        jumpe   1,kret_neg1
        move    1,6
        tlo     1,030002
        jrst    mfsproc_lookup_store
mfsproc_lookup_domain_root:
        move    1,6
        pushj   17,mfsdom_exists
        jumpe   1,kret_neg1
        move    1,6
        tlo     1,030002
        tro     1,0400000
        jrst    mfsproc_lookup_store
mfsproc_lookup_proc:
        caie    4,030002
        jrst    kret_neg1
        jumpn   0,mfsproc_lookup_domain_proc
        move    5,2
        move    7,3
        hrrz    6,1
        andi    6,0377                 ; low eight index bits are PID
        move    1,6
        pushj   17,mfsproc_proc_ptr
        jumpe   1,kret_neg1
        move    1,5
        movei   2,mfsproc_leaf_names
        movei   0,6
        pushj   17,mfsleaf_lookup
        jumpl   1,kret_neg1
        move    4,1
mfsproc_lookup_have_leaf:
        move    1,6
        lsh     4,010                  ; leaf selector into index bits 8..10
        ior     1,4
        tlo     1,030003               ; one uniform process-file kind
        jrst    mfsproc_lookup_store
mfsproc_lookup_domain_proc:
        move    5,2
        move    7,3
        move    6,1
        andi    6,0377
        move    1,5
        movei   2,mfsdom_leaf_names
        movei   0,6
        pushj   17,mfsleaf_lookup
        jumpl   1,kret_neg1
        move    4,1
mfsdom_lookup_found:
        move    1,6
        move    0,4
        lsh     0,010                  ; selector into index bits 8..
        ior     1,0
        tlo     1,030003               ; one uniform domain-file kind
        tro     1,0400000
        jrst    mfsproc_lookup_store
mfsproc_lookup_store:
        movem   1,(7)
        jrst    kret_zero

; int mfsproc_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
        .globl  mfsproc_readdir
mfsproc_readdir:
        jumpe   3,kret_neg1
        move    0,1
        andi    0,0400000
        hlrz    4,1
        caie    4,030001
        jrst    mfsproc_readdir_proc
        jumpn   0,mfsproc_readdir_domain_root
        skipn   5,proc_table
        jrst    kret_zero
        movei   6,0
        movei   7,0
        jrst    mfsproc_readdir_root_loop
mfsproc_readdir_root_loop:
        caml    6,proc_high_slot
        jrst    kret_zero
        hlrz    4,2(5)
        andi    4,PROC_STATE_LH_MASK
        jumpe   4,mfsproc_readdir_root_next
        camn    7,2
        jrst    mfsproc_readdir_root_found
        addi    7,1
mfsproc_readdir_root_next:
        addi    5,PROC_WORDS
        aoja    6,mfsproc_readdir_root_loop
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
        jrst    kret_zero
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
        jrst    kret_neg1
        jumpn   0,mfsproc_readdir_domain_proc
        move    7,3
        move    6,2
        hrrz    1,1
        andi    1,0377
        pushj   17,mfsproc_proc_ptr
        jumpe   1,kret_neg1
        movei   1,mfsproc_leaf_names
        move    2,6
        move    3,7
        movei   0,6
        jrst    mfsleaf_readdir
mfsproc_readdir_domain_proc:
        movei   1,mfsdom_leaf_names
        movei   0,6
        ; AC3 still holds the caller's dirent pointer.
        jrst    mfsleaf_readdir
mfsproc_readdir_store:
        movem   4,(7)
        movem   5,1(7)
        setzm   2(7)
        setzm   3(7)
        setzm   4(7)
        movem   6,5(7)
        jrst    kret_one

; int mfsproc_stat(vnode_t node, struct vfs_stat *st)
        .globl  mfsproc_stat
mfsproc_stat:
        jumpe   2,kret_neg1
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
        cain    5,030002               ; resolved process directory stays usable
        jrst    mfsproc_stat_dir       ; even after the process has disappeared
        move    6,1                    ; preserve vnode across proc lookup
        hrrz    1,6
        andi    1,0377
        pushj   17,mfsproc_proc_ptr
        jumpe   1,kret_neg1
mfsproc_stat_file:
        caie    5,030003
        jrst    kret_neg1
        hrrz    4,6                    ; leaf comes from original vnode
        lsh     4,-010
        andi    4,7
        caile   4,5                    ; six process leaves, 0..5
        jrst    kret_neg1
        movei   3,2
        movei   4,0444
        jrst    mfsproc_stat_store_zero
mfsproc_stat_domain:
        move    6,1
        andi    6,0377
        caie    5,030002
        jrst    mfsproc_stat_domain_status
        jrst    mfsproc_stat_dir
mfsproc_stat_domain_status:
        caie    5,030003
        jrst    kret_neg1
        movei   3,2
        movei   4,0444
        jrst    mfsproc_stat_store_zero
mfsproc_stat_store_zero:
        setz    5,
mfsproc_stat_store_words:
        movem   3,(7)
        movem   4,1(7)
        setzm   2(7)
        movem   5,3(7)
        jrst    kret_zero


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

; Provider-3 pseudo-files are native S6REC word streams.  Scalar leaves
; contain one record.  CMDLINE and ENVIRONMENT expose one record per original
; counted SIXBIT vector entry.  This keeps record boundaries explicit and
; removes the old character-at-offset renderer.
        .globl  mfsproc_read_words
        .globl  proc_scope_id
        .globl  kfmt_u18_sixbit

; Internal: emit one <=6-character SIXBIT word as one S6REC TEXT record.
; AC1=packed SIXBIT, AC2=chars, AC3=serialized word offset,
; AC4=destination, AC5=destination capacity. Return transferred words.
mfs_s6rec_emit_word:
        jumpe   4,kret_neg1
        jumpn   3,mfs_s6rec_emit_eof
        caige   5,2
        jrst    kret_neg1
        move    6,[010000000000]
        ior     6,2
        movem   6,(4)
        movem   1,1(4)
        movei   1,2
        popj    17,
mfs_s6rec_emit_eof:
        setz    1,
        popj    17,

; Internal: return one raw SIXBIT character from a trusted counted record.
; AC1=record address, AC2=character index. Return AC1=0..077.
mfsproc_image_record_sixchar:
        move    3,2
        idivi   3,6
        add     3,1
        move    1,1(3)
        imuli   4,6
        subi    4,036
        lsh     1,0(4)
        andi    1,077
        popj    17,

; Copy one counted SIXBIT record as one S6REC TEXT record.
; AC1=source, AC2=serialized offset, AC3=destination, AC4=capacity.
mfs_s6rec_copy_counted:
        jumpe   3,kret_neg1
        jumpn   2,mfs_s6rec_copy_eof
        move    5,(1)                  ; character count
        move    6,5
        addi    6,5
        idivi   6,6                    ; payload words
        move    7,6
        aoj     7,                     ; total S6REC words
        camle   7,4
        jrst    kret_neg1
        move    0,3                    ; BLT count+payload in one operation
        hrl     0,1
        move    4,3
        add     4,6
        blt     0,(4)
        move    5,(3)
        ior     5,[010000000000]
        movem   5,(3)
        move    1,7
        popj    17,
mfs_s6rec_copy_eof:
        setz    1,
        popj    17,

; Serialize basename(argv[0]) as one S6REC.  The source is already counted
; SIXBIT; only the possibly unaligned basename slice must be repacked.
; AC1=source record, AC2=destination, AC3=capacity.
mfs_s6rec_copy_name:
        add     17,kconst_4_4
        movei   0,-3(17)
        hrli    0,010
        blt     0,(17)
        move    010,1                  ; source
        move    011,2                  ; destination
        move    012,3                  ; capacity
        move    013,(1)                ; total chars
        move    5,013
        setz    6,                     ; basename start
mfs_s6rec_name_scan:
        sojl    5,mfs_s6rec_name_found
        move    1,010
        move    2,5
        pushj   17,mfsproc_image_record_sixchar
        caie    1,017                  ; SIXBIT '/'
        jrst    mfs_s6rec_name_scan
        movei   6,1(5)
mfs_s6rec_name_found:
        sub     013,6                  ; basename chars
        move    4,013
        addi    4,5
        idivi   4,6                    ; payload words
        move    0,4
        aoj     0,                     ; total record words
        camle   0,012
        jrst    mfs_s6rec_name_fail
        move    1,[010000000000]
        ior     1,013
        movem   1,(011)
        move    5,6                    ; source character index
        move    012,011
        aoj     012,                   ; destination payload pointer
        jumpe   013,mfs_s6rec_name_done
mfs_s6rec_name_word:
        setz    7,                     ; packed output word
        movei   6,6                    ; slots remaining
mfs_s6rec_name_char:
        move    1,010
        move    2,5
        pushj   17,mfsproc_image_record_sixchar
        lsh     7,6
        ior     7,1
        aoj     5,
        soj     6,
        soje    013,mfs_s6rec_name_partial
        jumpn   6,mfs_s6rec_name_char
        movem   7,(012)
        aoja    012,mfs_s6rec_name_word
mfs_s6rec_name_partial:
        imuli   6,6
        lsh     7,0(6)
        movem   7,(012)
mfs_s6rec_name_done:
        move    1,0
        jrst    mfs_s6rec_name_restore
mfs_s6rec_name_fail:
        seto    1,
mfs_s6rec_name_restore:
        movei   0,010
        hrli    0,-3(17)
        blt     0,013
        sub     17,kconst_4_4
        popj    17,

; Serialize NAME/CMDLINE/ENVIRONMENT directly from live startup metadata.
; AC1=validated proc *, AC2=leaf 3..5, AC3=serialized offset,
; AC4=destination, AC5=capacity.
mfsproc_image_read_words:
        add     17,kconst_5_5
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)
        move    010,2                  ; leaf
        move    011,3                  ; serialized offset
        move    012,4                  ; destination
        move    013,5                  ; capacity
        move    4,1(1)
        hrrz    014,4                  ; resident image base
        jumpe   014,mfsproc_image_words_eof
        hlrz    6,4
        subi    6,02000
        add     6,014                  ; argc,,envc metadata
        move    7,(6)
        hlrz    4,7                    ; argc
        hrrz    5,7                    ; envc
        aoj     6,                     ; argv vector
        cain    010,3
        jrst    mfsproc_image_words_name
        cain    010,4
        jrst    mfsproc_image_words_vector
        caie    010,5
        jrst    mfsproc_image_words_fail
        add     6,4                    ; environment vector
        move    4,5                    ; envc
mfsproc_image_words_vector:
        jumpe   4,mfsproc_image_words_eof
mfsproc_image_words_loop:
        move    1,(6)
        add     1,014                  ; counted record
        move    2,(1)
        addi    2,5
        idivi   2,6
        aoj     2,                     ; serialized words for this record
        jumpe   011,mfsproc_image_words_emit
        caml    011,2
        jrst    mfsproc_image_words_next
        jrst    mfsproc_image_words_fail
mfsproc_image_words_next:
        sub     011,2
        aoj     6,
        sojg    4,mfsproc_image_words_loop
        jrst    mfsproc_image_words_eof
mfsproc_image_words_emit:
        move    2,011
        move    3,012
        move    4,013
        pushj   17,mfs_s6rec_copy_counted
        jrst    mfsproc_image_words_done
mfsproc_image_words_name:
        jumpn   011,mfsproc_image_words_eof
        jumpe   4,mfsproc_image_words_eof
        move    1,(6)
        add     1,014
        move    2,012
        move    3,013
        pushj   17,mfs_s6rec_copy_name
        jrst    mfsproc_image_words_done
mfsproc_image_words_eof:
        setz    1,
        jrst    mfsproc_image_words_done
mfsproc_image_words_fail:
        seto    1,
mfsproc_image_words_done:
        movei   0,010
        hrli    0,-4(17)
        blt     0,014
        sub     17,kconst_5_5
        popj    17,

; int mfsproc_read_words(vnode_t node, kword_t off, kword_t *buf,
;     unsigned int nwords)
mfsproc_read_words:
        trne    1,0400000
        jrst    mfsdom_read_words
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,2                  ; serialized word offset
        move    011,3                  ; destination
        move    012,4                  ; capacity
        hlrz    4,1
        caie    4,030003
        jrst    mfsproc_read_words_fail
        move    013,1
        hrrz    1,1
        andi    1,0377
        pushj   17,mfsproc_proc_ptr
        jumpe   1,mfsproc_read_words_fail
        hrrz    2,013
        lsh     2,-010
        andi    2,7
        caige   2,3
        jrst    mfsproc_read_words_basic
        caile   2,5
        jrst    mfsproc_read_words_fail
        move    3,010
        move    4,011
        move    5,012
        pushj   17,mfsproc_image_read_words
        jrst    mfsproc_read_words_done
mfsproc_read_words_basic:
        cain    2,1
        jrst    mfsproc_read_words_state
        jumpe   2,mfsproc_read_words_ppid
        cain    2,2
        jrst    mfsproc_read_words_words
        jrst    mfsproc_read_words_fail
mfsproc_read_words_state:
        hrrz    2,013
        andi    2,0377
        pushj   17,mfsproc_state_word
        jrst    mfsproc_read_words_emit
mfsproc_read_words_ppid:
        ldb     1,[POINT 8,(1),27]
        pushj   17,kfmt_u18_sixbit
        jrst    mfsproc_read_words_emit
mfsproc_read_words_words:
        hlrz    1,1(1)
        pushj   17,kfmt_u18_sixbit
mfsproc_read_words_emit:
        move    3,010
        move    4,011
        move    5,012
        pushj   17,mfs_s6rec_emit_word
        jrst    mfsproc_read_words_done
mfsproc_read_words_fail:
        seto    1,
mfsproc_read_words_done:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .data
mfsdev_leaf_names:
        .word   0515700000000
        .word   2
        .word   0636441646300
        .word   5
        .word   0554555424562
        .word   0630000000007
        .word   0636741600000
        .word   4
mfsproc_leaf_names:
        .word   0606051440000
        .word   4
        .word   0636441644500
        .word   5
        .word   0675762446300
        .word   5
        .word   0564155450000
        .word   4
        .word   0435544545156
        .word   0450000000007
        .word   0455666516257
        .word   0565545566413
mfsdom_leaf_names:
        .word   0606257434563
        .word   0634563000011
        .word   0675762446300
        .word   5
        .word   0636741606045
        .word   0440000000007
        .word   0636741606757
        .word   0624463000011
        .word   0636457606045
        .word   0440000000007
        .word   0605144630000
        .word   4
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
; MonitorFS runtime state is implemented here directly; keep the compact
; target representation aligned with the public VFS contract.  This
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
        .globl  kret_zero
        .globl  kret_one
        .globl  kret_neg1
        .globl  proc_table
        .globl  proc_high_slot
        .globl  proc_swap_records
        .globl  proc_scope_id
        .globl  mfsproc_parse_slot
        .globl  mfsproc_format_slot

; AC1 = domain ID. Return AC1 = 1 if at least one active process belongs to
; the domain, otherwise zero.  Reuse the requested-metric scanner.
        .globl  mfsdom_exists
mfsdom_exists:
        ; Namespace walkers retain their loop state in AC3/AC6/AC7.
        push    17,3
        push    17,6
        push    17,7
        movei   2,0                    ; PROCESSES metric
        pushj   17,mfsdom_metric
        jumpl   1,mfsdom_exists_no
        movei   1,1
        jrst    mfsdom_exists_done
mfsdom_exists_no:
        setz    1,
mfsdom_exists_done:
        pop     17,7
        pop     17,6
        pop     17,3
        popj    17,

; AC1=domain id, AC2=metric selector 0..4.  Return the selected aggregate in
; AC1, or -1 when no active process belongs to the domain.
mfsdom_metric:
        move    5,1                    ; requested domain
        move    4,2                    ; selected metric
        setz    7,                     ; accumulator
        setz    0,                     ; seen flag
        skipn   6,proc_table
        jrst    mfsdom_metric_done
        setz    3,                     ; process slot
mfsdom_metric_loop:
        caml    3,proc_high_slot
        jrst    mfsdom_metric_done
        hlrz    2,2(6)
        andi    2,PROC_STATE_LH_MASK   ; process state in LH form
        jumpe   2,mfsdom_metric_next
        move    1,6
        pushj   17,proc_scope_id
        lsh     1,-DOMAIN_SHIFT
        andi    1,DOMAIN_MASK
        came    1,5
        jrst    mfsdom_metric_next
        movei   0,1
        jumpe   4,mfsdom_metric_inc
        hlrz    2,2(6)                 ; proc_scope_id may clobber AC2
        andi    2,PROC_STATE_LH_MASK
        cain    2,PROC_STATE_ZOMB_LH
        jrst    mfsdom_metric_next
        cain    4,4                    ; STOPPED
        jrst    mfsdom_metric_stopped
        caie    4,1                    ; WORDS
        jrst    mfsdom_metric_swap
        hrrz    1,1(6)
        jumpe   1,mfsdom_metric_next
        hlrz    1,1(6)
        add     7,1
        jrst    mfsdom_metric_next
mfsdom_metric_swap:
        caige   4,2
        jrst    mfsdom_metric_next
        caile   4,3
        jrst    mfsdom_metric_next
        hrrz    1,1(6)
        jumpn   1,mfsdom_metric_next
        jumpe   3,mfsdom_metric_next
        skipn   1,proc_swap_records
        jrst    mfsdom_metric_next
        add     1,3
        skipn   2,(1)
        jrst    mfsdom_metric_next
        cain    4,2
        jrst    mfsdom_metric_inc
        hrrz    2,2
        lsh     2,7                    ; swap blocks -> words
        add     7,2
        jrst    mfsdom_metric_next
mfsdom_metric_stopped:
        caie    2,PROC_STATE_STOP_LH
        jrst    mfsdom_metric_next
mfsdom_metric_inc:
        aoj     7,
mfsdom_metric_next:
        addi    6,PROC_WORDS
        aoja    3,mfsdom_metric_loop
mfsdom_metric_done:
        jumpe   0,kret_neg1
        move    1,7
        popj    17,

; Native S6REC word view for derived domain leaves.  Scalar metrics are
; one record.  PIDS is a stream of fixed four-character records ("ooo "), one
; per live member, so callers can walk it with the normal descriptor word
; offset without an auxiliary membership list.
mfsdom_read_words:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,2                  ; serialized word offset
        move    011,3                  ; destination
        move    012,4                  ; capacity
        hlrz    4,1
        caie    4,030003
        jrst    mfsdom_read_words_fail
        hrrz    4,1
        move    013,4
        andi    013,DOMAIN_MASK        ; domain ID
        lsh     4,-010
        andi    4,7                    ; leaf selector
        cain    4,5
        jrst    mfsdom_pids_read_words
        caile   4,4
        jrst    mfsdom_read_words_fail
        move    1,013
        move    2,4
        pushj   17,mfsdom_metric
        jumpl   1,mfsdom_read_words_fail
        pushj   17,kfmt_u18_sixbit
        move    3,010
        move    4,011
        move    5,012
        pushj   17,mfs_s6rec_emit_word
        jrst    mfsdom_read_words_done

mfsdom_pids_read_words:
        trne    010,1                  ; each PID record is exactly two words
        jrst    mfsdom_read_words_fail
        move    3,010
        lsh     3,-1                   ; requested member ordinal
        move    7,013                  ; requested domain
        skipn   6,proc_table
        jrst    mfsdom_read_words_eof
        setz    4,                     ; process slot
mfsdom_pids_words_scan:
        caml    4,proc_high_slot
        jrst    mfsdom_read_words_eof
        hlrz    2,2(6)
        andi    2,PROC_STATE_LH_MASK
        jumpe   2,mfsdom_pids_words_next
        move    1,6
        pushj   17,proc_scope_id
        lsh     1,-DOMAIN_SHIFT
        andi    1,DOMAIN_MASK
        came    1,7
        jrst    mfsdom_pids_words_next
        jumpe   3,mfsdom_pids_words_found
        subi    3,1
mfsdom_pids_words_next:
        addi    6,PROC_WORDS
        aoja    4,mfsdom_pids_words_scan
mfsdom_pids_words_found:
        move    1,4
        lsh     1,-6
        andi    1,7
        addi    1,020
        lsh     1,036                  ; first octal digit
        move    2,4
        lsh     2,-3
        andi    2,7
        addi    2,020
        lsh     2,030                  ; second octal digit
        ior     1,2
        move    2,4
        andi    2,7
        addi    2,020
        lsh     2,022                  ; third octal digit; fourth is space
        ior     1,2
        movei   2,4
        setz    3,
        move    4,011
        move    5,012
        pushj   17,mfs_s6rec_emit_word
        jrst    mfsdom_read_words_done
mfsdom_read_words_eof:
        setz    1,
        jrst    mfsdom_read_words_done
mfsdom_read_words_fail:
        seto    1,
mfsdom_read_words_done:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
