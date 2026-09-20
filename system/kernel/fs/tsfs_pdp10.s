; tsfs_pdp10.s -- read-only TSFS provider dispatcher.
        .text
        .globl  fs_mres_vector_dispatch
        .globl  pdp10_ret_zero
        .globl  tsfs_mres_dispatch
        .globl  tsfs_lookup
        .globl  tsfs_readdir
        .globl  tsfs_stat
        .globl  tsfs_parent
        .globl  tsfs_parent_name

tsfs_mres_dispatch:
        move    7,[tsfs_mres_vector]
        jrst    fs_mres_vector_dispatch

        .data
tsfs_mres_vector:
        .word   tsfs_lookup,,tsfs_readdir
        .word   tsfs_stat,,tsfs_parent
        .word   tsfs_parent_name,,0
        .word   0,,0
        .word   0,,0
        .word   0,,0
        .word   pdp10_ret_zero,,0
        .word   pdp10_ret_zero,,0
        .text

; Compact metadata record helpers called by tsfs.c.
;
; Per-mount location reuses DTFS' dtfs_media word because one VFS mount slot
; cannot be both DTFS and TSFS.  tsfs_file_shape supplies the second word:
;   dtfs_media[slot]       MEMBER,,START_BLOCK
;   tsfs_file_shape[slot]  BLOCK_COUNT,,RECORD_COUNT
;
; File records are eight words, hence 16 records per 128-word DECtape block.
; These routines intentionally contain only address decoding and I/O.  Media
; format/checksum validation remains in the transient userspace mount helper.
        .globl  dtfs_media
        .globl  tsfs_file_shape
        .globl  fs_block_workspace
        .globl  dtfs_dtc_read
        .globl  fs_copy_words
        .globl  tsfs_file_record
        .globl  tsfs_node_record

; int tsfs_file_record(vnode, index, record)
; AC1=vnode, AC2=index, AC3=8-word destination.  Return 0/-1 in AC1.
tsfs_file_record:
        push    17,010
        push    17,011
        move    010,2                  ; stable record index
        move    011,3                  ; stable destination

        hlrz    4,1                    ; provider,,kind/mount
        lsh     4,-6                   ; mount id into low six bits
        andi    4,077
        subi    4,1                    ; mount ids are one based
        hrrz    5,tsfs_file_shape(4)
        caml    010,5                  ; index >= record count
        jrst    tsfs_file_record_bad

        move    2,010
        lsh     2,-4                   ; table block = index / 16
        hrrz    5,dtfs_media(4)
        add     2,5                    ; physical DECtape block
        hlrz    1,dtfs_media(4)  ; physical DTC unit
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read
        jumpn   1,tsfs_file_record_done

        move    1,010
        andi    1,017
        lsh     1,3                    ; slot * eight words
        addi    1,fs_block_workspace
        move    2,011
        movei   3,010
        pushj   17,fs_copy_words
        setz    1,
        jrst    tsfs_file_record_done

tsfs_file_record_bad:
        hrroi   1,1
tsfs_file_record_done:
        pop     17,011
        pop     17,010
        popj    17,

; int tsfs_node_record(vnode, record, indexp)
; Root kind 1 maps to record zero when a table exists.  An empty filesystem
; has a zero table shape and returns +1 for its synthetic root.  Normal node
; kind 2 carries the nonzero file-record index in the vnode low half.
tsfs_node_record:
        push    17,010
        push    17,011
        move    010,2                  ; record destination
        move    011,3                  ; optional index result

        hlrz    4,1
        move    5,4
        andi    5,077                  ; local kind
        cain    5,1
        jrst    tsfs_node_record_root
        caie    5,2
        jrst    tsfs_node_record_bad
        hrrz    2,1                    ; ordinary record index
        jumpe   2,tsfs_node_record_bad
        jrst    tsfs_node_record_read

tsfs_node_record_root:
        lsh     4,-6
        andi    4,077
        subi    4,1
        skipn   tsfs_file_shape(4)
        jrst    tsfs_node_record_empty
        setz    2,                     ; root file record is zero

tsfs_node_record_read:
        push    17,2                   ; file_record clobbers scratch ACs
        move    3,010
        pushj   17,tsfs_file_record
        pop     17,4                   ; stable record index
        jumpn   1,tsfs_node_record_done
        jumpe   011,tsfs_node_record_ok
        movem   4,(011)
tsfs_node_record_ok:
        setz    1,
        jrst    tsfs_node_record_done

tsfs_node_record_empty:
        movei   1,1
        jrst    tsfs_node_record_done

tsfs_node_record_bad:
        hrroi   1,1
tsfs_node_record_done:
        pop     17,011
        pop     17,010
        popj    17,

; Shared non-root parent decode.
; AC1=node, AC2=8-word record buffer.  Return parent vnode in AC1 or -1.
tsfs_parent_common:
        hlrz    4,1
        andi    4,077
        caie    4,2                    ; TSFS_KIND_NODE only
        jrst    tsfs_parent_common_bad
        hrrz    4,1
        jumpe   4,tsfs_parent_common_bad
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        move    2,4
        move    3,011
        pushj   17,tsfs_file_record
        jumpn   1,tsfs_parent_common_done
        hlrz    2,(011)                ; parent record index
        hllz    1,010
        jumpn   2,tsfs_parent_common_node
        hlrz    3,1
        andi    3,0777700              ; provider + mount id
        iori    3,1                    ; TSFS_KIND_ROOT
        hrl     1,3
        jrst    tsfs_parent_common_done
tsfs_parent_common_node:
        hrr     1,2                    ; input already has NODE local kind
tsfs_parent_common_done:
        pop     17,011
        pop     17,010
        popj    17,
tsfs_parent_common_bad:
        hrroi   1,1
        popj    17,

; int tsfs_parent(vnode, vnode_t *parentp)
        .globl  tsfs_parent
tsfs_parent:
        jumpe   2,tsfs_parent_bad
        hlrz    4,1
        andi    4,077
        cain    4,1                    ; mounted root is its own parent
        jrst    tsfs_parent_root
        push    17,010
        move    010,2
        add     17,[010,,010]
        movei   2,-7(17)
        pushj   17,tsfs_parent_common
        jumpn   1,tsfs_parent_pop
        movem   1,(010)
        setz    1,
tsfs_parent_pop:
        sub     17,[010,,010]
        pop     17,010
        popj    17,
tsfs_parent_root:
        movem   1,(2)
        setz    1,
        popj    17,
tsfs_parent_bad:
        hrroi   1,1
        popj    17,

; int tsfs_parent_name(vnode, vnode_t *parentp, struct vfs_name *namep)
        .globl  tsfs_parent_name
        .globl  vfs_sixbit_name_chars
tsfs_parent_name:
        jumpe   2,tsfs_parent_name_bad
        jumpe   3,tsfs_parent_name_bad
        push    17,010
        push    17,011
        move    010,2                  ; parent result
        move    011,3                  ; name result
        add     17,[010,,010]
        movei   2,-7(17)
        pushj   17,tsfs_parent_common
        jumpn   1,tsfs_parent_name_pop
        movem   1,(010)
        movei   4,1(011)               ; name.words destination
        movei   5,-6(17)               ; record NAME0 source
        hrl     4,5
        blt     4,4(011)
        movei   1,1(011)
        movei   2,030
        pushj   17,vfs_sixbit_name_chars
        movem   1,(011)
        jumpe   1,tsfs_parent_name_fail
        setz    1,
        jrst    tsfs_parent_name_pop
tsfs_parent_name_fail:
        hrroi   1,1
tsfs_parent_name_pop:
        sub     17,[010,,010]
        pop     17,011
        pop     17,010
        popj    17,
tsfs_parent_name_bad:
        hrroi   1,1
        popj    17,

; int tsfs_stat(vnode, struct vfs_stat *st)
; Only fields represented by the current read-only TSFS V1 records are filled;
; uid/gid/mtime retain the same provider semantics as the former C routine.
        .globl  tsfs_stat
tsfs_stat:
        jumpe   2,tsfs_stat_bad
        push    17,010
        push    17,011
        move    010,1
        move    011,2
        add     17,[010,,010]
        movei   2,-7(17)
        setz    3,
        pushj   17,tsfs_node_record
        cain    1,1
        jrst    tsfs_stat_dir
        jumpn   1,tsfs_stat_pop_bad
        hrrz    4,-7(17)               ; file flags
        cain    4,1
        jrst    tsfs_stat_dir
        caie    4,2
        jrst    tsfs_stat_pop_bad

        movei   4,2                    ; VFS_TYPE_REG
        movem   4,0(011)
        movei   4,0444
        movem   4,1(011)
        move    4,0(17)                ; FILE_AUX = character size
        movem   4,2(011)
        move    4,-2(17)               ; FILE_SIZE_WORDS
        movem   4,3(011)
        setz    1,
        jrst    tsfs_stat_pop

tsfs_stat_dir:
        movei   4,1                    ; VFS_TYPE_DIR
        movem   4,0(011)
        movei   4,0555
        movem   4,1(011)
        setzm   2(011)
        setzm   3(011)
        setz    1,
        jrst    tsfs_stat_pop

tsfs_stat_pop_bad:
        hrroi   1,1
tsfs_stat_pop:
        sub     17,[010,,010]
        pop     17,011
        pop     17,010
        popj    17,
tsfs_stat_bad:
        hrroi   1,1
        popj    17,

; int tsfs_readdir(vnode dir, unsigned int off, struct vfs_dirent *ent)
; One eight-word scratch record is reused first for the directory and then its
; selected child.  The current V1 directory format stores a contiguous child
; range in FILE_AUX, so no resident iterator state is necessary.
        .globl  tsfs_readdir
tsfs_readdir:
        jumpe   3,tsfs_readdir_bad
        push    17,010
        push    17,011
        push    17,012
        move    010,1                  ; directory vnode
        move    011,2                  ; directory offset
        move    012,3                  ; output dirent
        add     17,[010,,010]

        movei   2,-7(17)
        setz    3,
        pushj   17,tsfs_node_record
        cain    1,1                    ; synthetic empty root
        jrst    tsfs_readdir_eof
        jumpn   1,tsfs_readdir_pop_bad
        hrrz    4,-7(17)
        caie    4,1                    ; TSFS_FILE_FLAG_DIR
        jrst    tsfs_readdir_pop_bad

        move    4,0(17)                ; FILE_AUX = FIRST,,COUNT
        hrrz    5,4
        caml    011,5
        jrst    tsfs_readdir_eof
        hlrz    2,4
        add     2,011
        move    1,010
        movei   3,-7(17)
        pushj   17,tsfs_file_record
        jumpn   1,tsfs_readdir_pop_bad

        movei   4,1(012)               ; ent->name.words
        movei   5,-6(17)               ; record NAME0
        hrl     4,5
        blt     4,4(012)
        movei   1,1(012)
        movei   2,030
        pushj   17,vfs_sixbit_name_chars
        movem   1,0(012)

        hrrz    4,-7(17)
        cain    4,1
        jrst    tsfs_readdir_dir
        caie    4,2
        jrst    tsfs_readdir_pop_bad
        movei   4,2                    ; VFS_TYPE_REG
        jrst    tsfs_readdir_type
tsfs_readdir_dir:
        movei   4,1                    ; VFS_TYPE_DIR
tsfs_readdir_type:
        movem   4,5(012)
        movei   1,1
        jrst    tsfs_readdir_pop

tsfs_readdir_eof:
        setz    1,
        jrst    tsfs_readdir_pop
tsfs_readdir_pop_bad:
        hrroi   1,1
tsfs_readdir_pop:
        sub     17,[010,,010]
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
tsfs_readdir_bad:
        hrroi   1,1
        popj    17,

; ---------------------------------------------------------------------------
; Mount and lookup policy.  The userspace handoff has already been fully
; validated; the kernel receives only FILE_LOC and FILE_SHAPE.  Re-check the
; physical bounds that resident metadata reads can dereference.
; ---------------------------------------------------------------------------
        .globl  vfs_mount
        .globl  vfs_name_valid
        .globl  fs_words_equal
        .globl  tsfs_mount_set
        .globl  tsfs_lookup

tsfs_mount_set:
        jumpe   1,tsfs_mount_bad
        jumpe   4,tsfs_mount_bad
        caie    3,1                    ; VFS_MOUNT_RDONLY
        jrst    tsfs_mount_bad
        push    17,010
        push    17,011
        push    17,012
        move    010,1                  ; two-word metadata handoff
        move    011,2                  ; mount target
        move    012,4                  ; returned root pointer

        move    5,0(010)               ; MEMBER,,START_BLOCK
        move    6,1(010)               ; BLOCKS,,RECORD_COUNT
        jumpe   5,tsfs_mount_empty
        jumpe   6,tsfs_mount_pop_bad
        hlrz    4,5
        caile   4,7                    ; valid DTC unit
        jrst    tsfs_mount_pop_bad
        hrrz    4,5
        caige   4,3                    ; descriptor/TDIR blocks are reserved
        jrst    tsfs_mount_pop_bad
        caile   4,01101                ; last physical DECtape block
        jrst    tsfs_mount_pop_bad
        jrst    tsfs_mount_do

tsfs_mount_empty:
        jumpn   6,tsfs_mount_pop_bad

tsfs_mount_do:
        push    17,0                   ; root scratch
        movei   1,(17)
        push    17,1                   ; sixth arg: rootp
        push    17,[1]                 ; fifth arg: VFS_MOUNT_RDONLY
        move    1,011
        movei   2,7                    ; TSFS_PROVIDER
        movei   3,1                    ; TSFS_KIND_ROOT
        setz    4,
        pushj   17,vfs_mount
        sub     17,[2,,2]
        jumpn   1,tsfs_mount_drop_bad

        hlrz    4,(17)                 ; provider,,kind/mount
        lsh     4,-6
        andi    4,077
        subi    4,1
        move    5,0(010)
        movem   5,dtfs_media(4)
        move    5,1(010)
        movem   5,tsfs_file_shape(4)
        move    5,(17)
        movem   5,(012)
        setz    1,
        jrst    tsfs_mount_drop

tsfs_mount_drop_bad:
        hrroi   1,1
tsfs_mount_drop:
        sub     17,[1,,1]
        jrst    tsfs_mount_pop

tsfs_mount_pop_bad:
        hrroi   1,1
tsfs_mount_pop:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
tsfs_mount_bad:
        hrroi   1,1
        popj    17,

; int tsfs_lookup(vnode dir, const struct vfs_name *name, vnode_t *nodep)
tsfs_lookup:
        jumpe   2,tsfs_lookup_bad
        jumpe   3,tsfs_lookup_bad
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        move    010,1                  ; directory vnode
        move    011,2                  ; requested component
        move    012,3                  ; result vnode pointer
        move    1,011
        pushj   17,vfs_name_valid
        jumpe   1,tsfs_lookup_pop_bad

        add     17,[010,,010]
        move    1,010
        movei   2,-7(17)
        setz    3,
        pushj   17,tsfs_node_record
        jumpn   1,tsfs_lookup_local_bad
        hrrz    4,-7(17)
        caie    4,1                    ; directory record required
        jrst    tsfs_lookup_local_bad
        move    4,0(17)                ; FILE_AUX = FIRST,,COUNT
        hlrz    013,4
        hrrz    014,4
        add     014,013                ; one-past child index

tsfs_lookup_loop:
        caml    013,014
        jrst    tsfs_lookup_local_bad
        move    1,010
        move    2,013
        movei   3,-7(17)
        pushj   17,tsfs_file_record
        jumpn   1,tsfs_lookup_local_bad
        movei   1,-6(17)               ; FILE_NAME0
        movei   2,1(011)               ; vfs_name.words
        movei   3,4
        pushj   17,fs_words_equal
        jumpn   1,tsfs_lookup_found
        aoja    013,tsfs_lookup_loop

tsfs_lookup_found:
        hllz    4,010
        tlz     4,077                   ; clear local kind
        tlo     4,2                     ; TSFS_KIND_NODE
        hrr     4,013
        movem   4,(012)
        setz    1,
        jrst    tsfs_lookup_local_done

tsfs_lookup_local_bad:
        hrroi   1,1
tsfs_lookup_local_done:
        sub     17,[010,,010]
        jrst    tsfs_lookup_pop

tsfs_lookup_pop_bad:
        hrroi   1,1
tsfs_lookup_pop:
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
tsfs_lookup_bad:
        hrroi   1,1
        popj    17,

        .bss
tsfs_file_shape:       .block 4        ; one shape word per VFS mount
