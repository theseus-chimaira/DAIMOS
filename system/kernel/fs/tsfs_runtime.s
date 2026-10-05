; tsfs_runtime.s -- read-only TSFS provider dispatcher.
        .text
        .globl  fs_mres_vector_dispatch
        .globl  kret_zero
        .globl  kret_neg1
        .globl  vfs_mount_prevalidated
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
        cain    6,025                    ; FS_MRES_OP_SPACE
        jrst    tsfs_space
        move    7,[tsfs_mres_vector]
        jrst    fs_mres_vector_dispatch

; TSFS is immutable.  Report the complete DECtape member span as both total
; and used so available space is correctly zero rather than implying writable
; capacity which TSFS cannot allocate.
tsfs_space:
        movei   1,01102
        lsh     1,7
        move    2,1
        popj    17,

        .data
tsfs_mres_vector:
        .word   tsfs_lookup,,tsfs_readdir
        .word   tsfs_stat,,tsfs_parent
        .word   tsfs_parent_name,,0
        .word   0,,0
        .word   0,,0
        .word   0,,0
        .word   tsfs_read_words,,0
        .word   kret_zero,,0
        .text

; Compact metadata helpers.
;
; Per-mount location reuses DTFS' dtfs_media word because one VFS mount slot
; cannot be both DTFS and TSFS.  tsfs_file_shape supplies the second word:
;   dtfs_media[slot]       (DTC_UNIT << 15 | RECORD_COUNT),,START_BLOCK
;   tsfs_file_shape[slot]  packed logical->physical DTC map
;   tsfs_extent_media[slot] (DTC_UNIT << 15 | RECORD_COUNT),,START_BLOCK
;
; Providers are serialized around fs_block_workspace.  Therefore metadata
; records can be consumed directly from that workspace until the next DTC
; read; copying each eight-word record onto a kernel stack only wastes RAM and
; time.  Media checksum/structural validation remains in transient userspace.
        .globl  dtfs_media
        .globl  tsfs_file_shape
        .globl  tsfs_extent_media
        .globl  fs_block_workspace
        .globl  dtfs_dtc_read
        .globl  bcache_reclaim
        .globl  tsfs_file_record
        .globl  tsfs_node_record

; kword_t *tsfs_file_record(vnode, index)
; kword_t *tsfs_extent_record(vnode, index)
; Both fixed-record tables use the same locator format.  AC3 selects the
; per-mount locator array; AC6 packs BLOCK_SHIFT,,SLOT_MASK.  Record-word
; shift is 7 + BLOCK_SHIFT because every DTC block contains 128 words.
tsfs_file_record:
        movei   3,dtfs_media
        move    6,[-4,,017]            ; 16 eight-word records per block
        jrst    tsfs_table_record

tsfs_extent_record:
        movei   3,tsfs_extent_media
        move    6,[-5,,037]            ; 32 four-word records per block

tsfs_table_record:
        push    17,010
        move    010,2                  ; stable record index
        hlrz    4,1
        lsh     4,-6
        andi    4,077
        subi    4,1
        add     3,4                    ; selected mount locator
        hlrz    5,(3)
        andi    5,077777               ; 15-bit record count
        caml    010,5
        jrst    tsfs_table_record_bad

        move    2,010
        hlre    7,6                    ; negative log2(records/block)
        lsh     2,0(7)
        hrrz    5,(3)
        add     2,5                    ; physical table block
        hlrz    1,(3)
        lsh     1,-017                 ; physical DTC unit
        movei   3,fs_block_workspace
        ; DTC I/O uses AC4-AC7.  Preserve the packed table shape in AC6
        ; because the record offset is derived from it after the read.
        push    17,6
        pushj   17,dtfs_dtc_read
        pop     17,6
        jumpn   1,tsfs_table_record_done

        move    1,010
        hrrz    7,6
        and     1,7                    ; slot inside table block
        hlre    7,6
        addi    7,7                    ; log2(record words)
        lsh     1,0(7)
        addi    1,fs_block_workspace
        jrst    tsfs_table_record_done

tsfs_table_record_bad:
        hrroi   1,1
tsfs_table_record_done:
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
        jrst    kret_neg1
        hrrz    2,1                    ; ordinary record index
        jumpe   2,kret_neg1
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
        jrst    kret_neg1
        hrrz    2,1
        jumpe   2,kret_neg1
        push    17,010
        move    010,1
        pushj   17,tsfs_file_record
        jumpl   1,tsfs_parent_common_pop
        move    5,1                    ; record pointer
        hlrz    2,(5)                  ; parent record index
        jumpn   2,tsfs_parent_common_node
        movsi   1,070001               ; provider 7, local ROOT kind
        jrst    tsfs_parent_common_ok

tsfs_parent_common_node:
        movsi   1,070002               ; provider 7, local NODE kind
        hrr     1,2

tsfs_parent_common_ok:
        move    2,5                    ; record pointer for parent_name

tsfs_parent_common_pop:
        pop     17,010
        popj    17,

; int tsfs_parent(vnode, vnode_t *parentp)
        .globl  tsfs_parent
tsfs_parent:
        jumpe   2,kret_neg1
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
        jumpe   2,kret_neg1
        jumpe   3,kret_neg1
        push    17,010
        push    17,011
        move    010,2                  ; parent result
        move    011,3                  ; name result
        pushj   17,tsfs_parent_common
        jumpn   1,tsfs_parent_name_pop
        movem   1,(010)
        movei   1,1(2)                 ; record NAME0 source
        move    2,011                   ; struct vfs_name *
        pushj   17,vfs_name_from_words
        move    1,(011)
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
; vfs_stat() has already zeroed UID/GID/MTIME.  TSFS type flags deliberately
; equal VFS DIR/REG values, so one masked value serves both validation/store.
        .globl  tsfs_stat
tsfs_stat:
        jumpe   2,kret_neg1
        push    17,010
        move    010,2
        pushj   17,tsfs_node_record
        cain    1,1
        jrst    tsfs_stat_synthetic_root
        jumpl   1,tsfs_stat_pop_bad
        move    7,1                     ; record pointer
        hrrz    4,(7)                   ; flags + mode
        move    5,4
        andi    5,3                     ; DIR=1, REG=2 == VFS type
        jumpe   5,tsfs_stat_pop_bad
        caile   5,2
        jrst    tsfs_stat_pop_bad
        movem   5,0(010)                ; st->type
        lsh     4,-6
        andi    4,07777
        movem   4,1(010)                ; st->mode
        setzm   2(010)                  ; reserved
        move    6,5(7)                  ; dir owner / regular size
        cain    5,1
        jrst    tsfs_stat_dir_record
        movem   6,3(010)                ; regular size
        move    6,7(7)                  ; regular owner
        jrst    tsfs_stat_attrs
tsfs_stat_dir_record:
        setzm   3(010)                  ; directory size
tsfs_stat_attrs:
        move    5,6
        lsh     5,-011
        andi    5,0777
        movem   5,4(010)                ; uid
        andi    6,0777
        movem   6,5(010)                ; gid
        setz    1,
        jrst    tsfs_stat_pop

tsfs_stat_synthetic_root:
        movei   4,1
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
        jumpe   3,kret_neg1
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
        andi    4,3
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
        andi    011,3

        movei   1,1(5)                 ; record NAME0
        move    2,012                   ; ent->name
        pushj   17,vfs_name_from_words

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
; TSFS V1 regular data uses fixed 0400-word restart extents. The transient
; validator guarantees exact logical coverage. Each extent is either STORED
; (flag 0, one/two physical blocks) or D6LZ (flag 1, exactly one physical
; block with a two-word restart header). Direct restart indexing avoids a
; resident extent walk. D6LZ output is transient managed core and is freed
; before return.
        .globl  mm_alloc
        .globl  mm_free
        .globl  d6lz36_decode_buffer
        .equ    TSFS_EXT_STORED,0
        .equ    TSFS_EXT_D6LZ,1
        .equ    TSFS_RESTART_WORDS,0400
        .equ    TSFS_DECODE_OWNER,011

tsfs_read_words:
        jumpe   4,kret_zero
        ; AC10..AC16 form one contiguous callee-save block.
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)
        move    010,3
        move    011,2
        move    012,4
        move    013,1
        pushj   17,tsfs_node_record
        caig    1,1
        jrst    tsfs_read_words_fail0
        hrrz    5,(1)
        andi    5,3
        caie    5,2
        jrst    tsfs_read_words_fail0
        move    5,5(1)
        caml    011,5
        jrst    tsfs_read_words_eof0
        sub     5,011
        camle   012,5
        move    012,5
        move    6,6(1)
        hlrz    014,6
        hrrz    015,6
        jumpe   015,tsfs_read_words_fail0
        push    17,012                  ; total clipped count
        hlrz    4,013
        lsh     4,-6
        andi    4,077
        subi    4,1
        move    016,tsfs_file_shape(4)

tsfs_read_restart:
        move    2,011
        lsh     2,-010                  ; restart number = offset / 0400
        caml    2,015
        jrst    tsfs_read_words_fail
        add     2,014
        move    1,013
        pushj   17,tsfs_extent_record
        jumpl   1,tsfs_read_words_fail
        move    6,1(1)
        hlrz    5,6
        imuli   5,3
        movn    5,5
        move    7,016
        lsh     7,0(5)
        andi    7,7
        hrlz    7,7
        hrr     7,6
        move    6,2(1)
        hlrz    5,6
        cain    5,TSFS_EXT_D6LZ
        jrst    tsfs_read_d6lz
        jumpn   5,tsfs_read_words_fail

tsfs_read_stored:
        move    5,011
        andi    5,0377
        move    6,5
        lsh     6,-7
        add     7,6
        andi    5,0177
        hlrz    1,7
        hrrz    2,7
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read
        jumpn   1,tsfs_read_words_fail
        move    5,011
        andi    5,0177
        movei   6,0200
        sub     6,5
        camle   6,012
        move    6,012
        movei   7,0400
        move    1,011
        andi    1,0377
        sub     7,1
        camle   6,7
        move    6,7
        movei   1,fs_block_workspace
        add     1,5
        move    2,010
        move    3,6
        pushj   17,fs_copy_words
        add     010,6
        add     011,6
        sub     012,6
        jumpn   012,tsfs_read_restart
        jrst    tsfs_read_words_done

tsfs_read_d6lz:
        hrrz    6,6
        caie    6,1
        jrst    tsfs_read_words_fail
        hlrz    1,7
        hrrz    2,7
        movei   3,fs_block_workspace
        pushj   17,dtfs_dtc_read
        jumpn   1,tsfs_read_words_fail
        move    6,fs_block_workspace
        jumpe   6,tsfs_read_words_fail
        caile   6,TSFS_RESTART_WORDS
        jrst    tsfs_read_words_fail
        move    7,fs_block_workspace+1
        jumpe   7,tsfs_read_words_fail
        caile   7,0176
        jrst    tsfs_read_words_fail
        push    17,[0]
        movei   5,(17)
        push    17,5
        move    1,6
        movei   2,3
        movei   3,TSFS_DECODE_OWNER
        setz    4,
        pushj   17,mm_alloc
        sub     17,kconst_1_1
        jumpn   1,tsfs_read_d6lz_drop
        ; The common runtime frontend owns decoder-state save/restore and exact
        ; compressed-buffer consumption.  The allocation result lives in our
        ; current stack word; pack its address with the decoded restart length.
        movei   2,fs_block_workspace+2
        move    3,fs_block_workspace+1
        move    4,(17)
        hrl     4,fs_block_workspace
        pushj   17,d6lz36_decode_buffer
        jumpn   1,tsfs_read_d6lz_free
        move    5,011
        andi    5,0377
        move    6,fs_block_workspace
        sub     6,5
        camle   6,012
        move    6,012
        move    1,(17)
        add     1,5
        move    2,010
        move    3,6
        pushj   17,fs_copy_words
        add     010,6
        add     011,6
        sub     012,6
        move    1,(17)
        movei   2,3
        movei   3,TSFS_DECODE_OWNER
        pushj   17,mm_free
        sub     17,kconst_1_1
        jumpn   1,tsfs_read_words_fail
        jumpn   012,tsfs_read_restart
        jrst    tsfs_read_words_done

tsfs_read_d6lz_free:
        move    1,(17)
        movei   2,3
        movei   3,TSFS_DECODE_OWNER
        pushj   17,mm_free
tsfs_read_d6lz_drop:
        sub     17,kconst_1_1
        jrst    tsfs_read_words_fail

tsfs_read_words_done:
        move    1,(17)
        sub     17,kconst_1_1
        jrst    tsfs_read_words_restore

tsfs_read_words_fail:
        sub     17,kconst_1_1
tsfs_read_words_fail0:
        hrroi   1,1
        jrst    tsfs_read_words_restore

tsfs_read_words_eof0:
        setz    1,

tsfs_read_words_restore:
        movei   0,-6(17)
        hrl     0,0
        hrri    0,010
        blt     0,016
        sub     17,kconst_7_7
        popj    17,

tsfs_mount_set:
        caie    3,1                    ; VFS_MOUNT_RDONLY
        jrst    kret_neg1
        jumpe   4,kret_neg1       ; provider ABI requires rootp
        push    17,010
        push    17,011
        push    17,012
        move    010,1                  ; three-word metadata handoff
        move    011,2                  ; mount target
        move    012,4                  ; returned root pointer

        move    5,0(010)               ; packed FILE state
        move    6,1(010)               ; packed member-map state
        jumpe   5,tsfs_mount_empty
        setz    6,                     ; 0=FILE record, 1=EXTENT record
tsfs_mount_validate_media:
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
        jumpn   6,tsfs_mount_do
        move    5,2(010)               ; packed EXTENT state or zero
        jumpe   5,tsfs_mount_do
        aoja    6,tsfs_mount_validate_media

tsfs_mount_empty:
        jumpn   6,tsfs_mount_pop_bad
        move    5,2(010)
        jumpn   5,tsfs_mount_pop_bad

tsfs_mount_do:
        push    17,0                   ; root scratch
        movei   1,(17)
        push    17,1                   ; sixth arg: rootp
        push    17,[1]                 ; fifth arg: VFS_MOUNT_RDONLY
        move    1,011
        movei   2,7                    ; TSFS_PROVIDER
        movei   3,1                    ; TSFS_KIND_ROOT
        setz    4,
        pushj   17,vfs_mount_prevalidated
        sub     17,kconst_2_2
        jumpn   1,tsfs_mount_drop_bad

        hlrz    4,(17)                 ; provider,,kind/mount
        lsh     4,-6
        andi    4,077
        subi    4,1
        move    5,0(010)
        movem   5,dtfs_media(4)
        move    5,1(010)
        movem   5,tsfs_file_shape(4)
        move    5,2(010)
        movem   5,tsfs_extent_media(4)
        pushj   17,bcache_reclaim       ; accepted removable-media handoff
        move    5,(17)
        movem   5,(012)
        setz    1,
        jrst    tsfs_mount_drop

tsfs_mount_drop_bad:
        hrroi   1,1
tsfs_mount_drop:
        sub     17,kconst_1_1
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
        jumpe   2,kret_neg1
        jumpe   3,kret_neg1
        add     17,kconst_5_5
        movei   0,-4(17)
        hrli    0,010
        blt     0,(17)
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
        andi    4,3
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
        pushj   17,vfs_name_words_equal
        jumpn   1,tsfs_lookup_found
        aoja    013,tsfs_lookup_loop

tsfs_lookup_found:
        hrrz    4,013
        tlo     4,070002                ; provider 7, local NODE kind
        movem   4,(012)
        setz    1,
        jrst    tsfs_lookup_pop

tsfs_lookup_local_bad:
        hrroi   1,1
        jrst    tsfs_lookup_pop

tsfs_lookup_pop_bad:
        hrroi   1,1
tsfs_lookup_pop:
        movei   0,010
        hrli    0,-4(17)
        blt     0,014
        sub     17,kconst_5_5
        popj    17,

        .bss
tsfs_file_shape:       .block 4        ; logical->physical map per mount
tsfs_extent_media:      .block 4        ; extent-table locator per mount
