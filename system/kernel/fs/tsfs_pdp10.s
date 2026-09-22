; tsfs_pdp10.s -- read-only TSFS provider dispatcher.
        .text
        .globl  fs_mres_vector_dispatch
        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1
        .globl  tsfs_mres_dispatch
        .globl  tsfs_lookup
        .globl  tsfs_readdir
        .globl  tsfs_stat
        .globl  tsfs_parent
        .globl  tsfs_parent_name
        .globl  tsfs_read_words

tsfs_mres_dispatch:
        cain    6,022                    ; FS_MRES_OP_MOUNT_UNIT
        jrst    tsfs_mount_set
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
        .word   tsfs_read_words,,0
        .word   pdp10_ret_zero,,0
        .text

; Compact metadata helpers.
;
; Per-mount location reuses DTFS' dtfs_media word because one VFS mount slot
; cannot be both DTFS and TSFS.  tsfs_file_shape supplies the second word:
;   dtfs_media[slot]       (DTC_UNIT << 15 | RECORD_COUNT),,START_BLOCK
;   tsfs_file_shape[slot]  packed logical->physical DTC map
;
; Providers are serialized around fs_block_workspace.  Therefore metadata
; records can be consumed directly from that workspace until the next DTC
; read; copying each eight-word record onto a kernel stack only wastes RAM and
; time.  Media checksum/structural validation remains in transient userspace.
        .globl  dtfs_media
        .globl  tsfs_file_shape
        .globl  fs_block_workspace
        .globl  dtfs_dtc_read
        .globl  tsfs_file_record
        .globl  tsfs_node_record

; kword_t *tsfs_file_record(vnode, index)
; Return pointer into fs_block_workspace, or -1.
tsfs_file_record:
        push    17,010
        move    010,2                  ; stable record index
        hlrz    4,1                    ; provider,,kind/mount
        lsh     4,-6                   ; mount id into low six bits
        andi    4,077
        subi    4,1                    ; mount ids are one based
        hlrz    5,dtfs_media(4)
        andi    5,077777               ; 15-bit record count
        caml    010,5                  ; index >= record count
        jrst    tsfs_file_record_bad

        move    2,010
        lsh     2,-4                   ; table block = index / 16
        hrrz    5,dtfs_media(4)
        add     2,5                    ; physical DECtape block
        hlrz    1,dtfs_media(4)
        lsh     1,-017                 ; physical DTC unit (15 decimal)
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read
        jumpn   1,tsfs_file_record_done

        move    1,010
        andi    1,017
        lsh     1,3                    ; slot * eight words
        addi    1,fs_block_workspace
        jrst    tsfs_file_record_done

tsfs_file_record_bad:
        hrroi   1,1
tsfs_file_record_done:
        pop     17,010
        popj    17,

; kword_t *tsfs_node_record(vnode)
; Root kind 1 maps to record zero when a table exists.  An empty filesystem
; returns +1 for its synthetic root.  Normal node kind 2 carries the nonzero
; file-record index in the vnode low half.  Return pointer, +1, or -1.
tsfs_node_record:
        hlrz    4,1
        move    5,4
        andi    5,077                  ; local kind
        cain    5,1
        jrst    tsfs_node_record_root
        caie    5,2
        jrst    pdp10_ret_neg1
        hrrz    2,1                    ; ordinary record index
        jumpe   2,pdp10_ret_neg1
        jrst    tsfs_file_record

tsfs_node_record_root:
        lsh     4,-6
        andi    4,077
        subi    4,1
        hlrz    5,dtfs_media(4)
        andi    5,077777
        jumpe   5,tsfs_node_record_empty
        setz    2,                     ; root file record is zero
        jrst    tsfs_file_record

tsfs_node_record_empty:
        movei   1,1
        popj    17,

; Shared non-root parent decode.
; AC1=node.  Return parent vnode in AC1 and the source record pointer in AC2.
tsfs_parent_common:
        hlrz    4,1
        andi    4,077
        caie    4,2                    ; TSFS_KIND_NODE only
        jrst    pdp10_ret_neg1
        hrrz    2,1
        jumpe   2,pdp10_ret_neg1
        push    17,010
        move    010,1
        pushj   17,tsfs_file_record
        jumpl   1,tsfs_parent_common_pop
        move    5,1                    ; record pointer
        hlrz    2,(5)                  ; parent record index
        hllz    1,010
        jumpn   2,tsfs_parent_common_node
        hlrz    3,1
        andi    3,0777700              ; provider + mount id
        iori    3,1                    ; TSFS_KIND_ROOT
        hrl     1,3
        jrst    tsfs_parent_common_ok

tsfs_parent_common_node:
        hrr     1,2                    ; input already has NODE local kind

tsfs_parent_common_ok:
        move    2,5                    ; record pointer for parent_name

tsfs_parent_common_pop:
        pop     17,010
        popj    17,

; int tsfs_parent(vnode, vnode_t *parentp)
        .globl  tsfs_parent
tsfs_parent:
        jumpe   2,pdp10_ret_neg1
        hlrz    4,1
        andi    4,077
        cain    4,1                    ; mounted root is its own parent
        jrst    tsfs_parent_root
        push    17,010
        move    010,2
        pushj   17,tsfs_parent_common
        jumpn   1,tsfs_parent_pop
        movem   1,(010)
        setz    1,
tsfs_parent_pop:
        pop     17,010
        popj    17,
tsfs_parent_root:
        movem   1,(2)
        setz    1,
        popj    17,
; int tsfs_parent_name(vnode, vnode_t *parentp, struct vfs_name *namep)
        .globl  tsfs_parent_name
        .globl  vfs_sixbit_name_chars
tsfs_parent_name:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
        push    17,010
        push    17,011
        move    010,2                  ; parent result
        move    011,3                  ; name result
        pushj   17,tsfs_parent_common
        jumpn   1,tsfs_parent_name_pop
        movem   1,(010)
        movei   4,1(011)               ; name.words destination
        movei   5,1(2)                 ; record NAME0 source
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
        pop     17,011
        pop     17,010
        popj    17,
; int tsfs_stat(vnode, struct vfs_stat *st)
        .globl  tsfs_stat
tsfs_stat:
        jumpe   2,pdp10_ret_neg1
        push    17,010
        move    010,2
        pushj   17,tsfs_node_record
        cain    1,1
        jrst    tsfs_stat_dir
        jumpl   1,tsfs_stat_pop_bad
        hrrz    4,(1)                  ; file flags
        cain    4,1
        jrst    tsfs_stat_dir
        caie    4,2
        jrst    tsfs_stat_pop_bad

        movei   4,2                    ; VFS_TYPE_REG
        movem   4,0(010)
        movei   4,0444
        movem   4,1(010)
        move    4,5(1)                 ; FILE_SIZE_WORDS
        movem   4,3(010)
        lsh     4,2                    ; current word files carry four chars
        movem   4,2(010)
        setz    1,
        jrst    tsfs_stat_pop

tsfs_stat_dir:
        movei   4,1                    ; VFS_TYPE_DIR
        movem   4,0(010)
        movei   4,0555
        movem   4,1(010)
        setzm   2(010)
        setzm   3(010)
        setz    1,
        jrst    tsfs_stat_pop

tsfs_stat_pop_bad:
        hrroi   1,1
tsfs_stat_pop:
        pop     17,010
        popj    17,
; int tsfs_readdir(vnode dir, unsigned int off, struct vfs_dirent *ent)
tsfs_readdir:
        jumpe   3,pdp10_ret_neg1
        push    17,010
        push    17,011
        push    17,012
        move    010,1                  ; directory vnode
        move    011,2                  ; directory offset
        move    012,3                  ; output dirent
        pushj   17,tsfs_node_record
        cain    1,1                    ; synthetic empty root
        jrst    tsfs_readdir_eof
        jumpl   1,tsfs_readdir_pop_bad
        hrrz    4,(1)
        caie    4,1                    ; TSFS_FILE_FLAG_DIR
        jrst    tsfs_readdir_pop_bad

        move    4,7(1)                 ; FILE_AUX = FIRST,,COUNT
        hrrz    5,4
        caml    011,5
        jrst    tsfs_readdir_eof
        hlrz    2,4
        add     2,011
        move    1,010
        pushj   17,tsfs_file_record
        jumpl   1,tsfs_readdir_pop_bad
        move    5,1                    ; child record pointer
        hrrz    011,(5)                ; preserve child flags across call

        movei   4,1(012)               ; ent->name.words
        movei   3,1(5)                 ; record NAME0
        hrl     4,3
        blt     4,4(012)
        movei   1,1(012)
        movei   2,030
        pushj   17,vfs_sixbit_name_chars
        movem   1,0(012)

        cain    011,1
        jrst    tsfs_readdir_dir
        caie    011,2
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
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
; int tsfs_read_words(vnode node, unsigned int off, kword_t *buf,
;     unsigned int nwords)
; V1 mount validation guarantees exactly one contiguous tape-local extent for
; every nonempty regular file.  FILE_AUX caches that extent's logical
; member,,start-block; transient userspace verifies the cache against the
; checksummed canonical extent table before handing the mount to the kernel.
tsfs_read_words:
        jumpe   4,pdp10_ret_zero
        add     17,[5,,5]
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)                  ; save AC10..AC14
        move    010,3                   ; destination base
        move    011,2                   ; file word offset
        move    012,4                   ; requested count
        move    013,1                   ; vnode until mount decode

        pushj   17,tsfs_node_record
        caig    1,1                    ; pointer must exceed synthetic root
        jrst    tsfs_read_words_fail

        hrrz    5,(1)
        caie    5,2                    ; regular file only
        jrst    tsfs_read_words_fail
        move    5,5(1)                 ; file size
        caml    011,5
        jrst    tsfs_read_words_eof
        sub     5,011
        camle   012,5
        move    012,5                  ; total=min(request,available)
        move    6,7(1)                 ; inline member,,start block

        hlrz    4,013
        lsh     4,-6
        andi    4,077
        subi    4,1
        move    013,tsfs_file_shape(4) ; logical->physical DTC map

        hlrz    5,6                    ; logical member
        imuli   5,3
        movn    5,5
        lsh     013,0(5)
        andi    013,7                  ; physical DTC unit
        hrlz    013,013                ; physical unit,,0
        hrr     013,6                  ; physical unit,,extent start block
        move    5,011
        lsh     5,-7
        add     013,5                  ; first physical data block
        andi    011,0177               ; first-block word offset
        move    014,012                ; return count after clipping

tsfs_read_words_loop:
        hlrz    1,013
        hrrz    2,013
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read
        jumpn   1,tsfs_read_words_fail
        movei   6,0200
        sub     6,011                  ; available in block
        camle   6,012
        move    6,012                  ; take=min(block room, remaining)
        movei   1,fs_block_workspace
        add     1,011
        move    2,010
        move    3,6
        pushj   17,fs_copy_words
        add     010,6                  ; advance destination
        sub     012,6                  ; consume request
        jumpe   012,tsfs_read_words_done
        setz    011,
        aos     013                    ; next block, preserve unit in LH
        jrst    tsfs_read_words_loop

tsfs_read_words_done:
        move    1,014
        jrst    tsfs_read_words_restore

tsfs_read_words_eof:
        setz    1,
        jrst    tsfs_read_words_restore

tsfs_read_words_fail:
        hrroi   1,1

tsfs_read_words_restore:
        movei   0,010
        hrli    0,-4(17)
        blt     0,014
        sub     17,[5,,5]
        popj    17,

; ---------------------------------------------------------------------------
; Mount and lookup policy.  The userspace handoff has already been fully
; validated; the kernel receives two packed runtime words.  Re-check only the
; physical file-table address that resident metadata reads can dereference.
; ---------------------------------------------------------------------------
        .globl  vfs_mount
        .globl  vfs_name_valid
        .globl  fs_words_equal
        .globl  tsfs_mount_set
        .globl  tsfs_lookup

tsfs_mount_set:
        caie    3,1                    ; VFS_MOUNT_RDONLY
        jrst    pdp10_ret_neg1
        push    17,010
        push    17,011
        push    17,012
        move    010,1                  ; two-word metadata handoff
        move    011,2                  ; mount target
        move    012,4                  ; returned root pointer

        move    5,0(010)               ; packed FILE state
        move    6,1(010)               ; packed member-map state
        jumpe   5,tsfs_mount_empty
        hlrz    4,5
        move    7,4
        andi    7,077777               ; 15-bit record count
        jumpe   7,tsfs_mount_pop_bad
        lsh     4,-017                 ; DTC unit (15 decimal)
        caile   4,7
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
; int tsfs_lookup(vnode dir, const struct vfs_name *name, vnode_t *nodep)
tsfs_lookup:
        jumpe   2,pdp10_ret_neg1
        jumpe   3,pdp10_ret_neg1
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

        move    1,010
        pushj   17,tsfs_node_record
        caig    1,1                    ; reject error/synthetic root
        jrst    tsfs_lookup_local_bad
        hrrz    4,(1)
        caie    4,1                    ; directory record required
        jrst    tsfs_lookup_local_bad
        move    4,7(1)                 ; FILE_AUX = FIRST,,COUNT
        hlrz    013,4
        hrrz    014,4
        add     014,013                ; one-past child index

tsfs_lookup_loop:
        caml    013,014
        jrst    tsfs_lookup_local_bad
        move    1,010
        move    2,013
        pushj   17,tsfs_file_record
        jumpl   1,tsfs_lookup_local_bad
        movei   5,1(1)                 ; FILE_NAME0
        move    1,5
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
        jrst    tsfs_lookup_pop

tsfs_lookup_local_bad:
        hrroi   1,1
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

        .bss
tsfs_file_shape:       .block 4        ; one shape word per VFS mount
