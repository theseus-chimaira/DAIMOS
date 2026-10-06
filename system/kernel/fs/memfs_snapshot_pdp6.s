/**
 * @file memfs_snapshot_pdp6.s
 * @brief Compact PDP-6 orderly-shutdown MEMFS persistence.
 *
 * Persistence is cold control-path code.  Keep the portable C implementation
 * as the reference, but avoid carrying KCC's large loop/save frames in the
 * resident PDP-6 package.
 */

        .text
        .globl  memfs_snapshot_mount
        .globl  memfs_snapshot_shutdown
        .globl  memfs_data_ensure
        .globl  memfs_mres_fs
        .globl  fs_block_workspace
        .globl  backstore_enabled
        .globl  backstore_blocks
        .globl  backstore_blocks_used
        .globl  backstore_bitmap
        .globl  backstore_read
        .globl  backstore_write

        .equ    SNAP_MAGIC,055464663
        .equ    SNAP_VERSION,2
        .equ    SNAP_META_WORDS,01200
        .equ    SNAP_META_BLOCKS,5
        .equ    SECTOR_WORDS,0200
        .equ    MEMFS_MOUNT_PERSIST,2
        .equ    VFS_TYPE_REG,2

        .equ    MEMFS_NODES,0
        .equ    MEMFS_POOL,1
        .equ    MEMFS_POOL_WORDS,2
        .equ    MEMFS_USED_WORDS,3
        .equ    NODE_META,5
        .equ    NODE_DATA,6

; AC1=words, return AC1=36-bit additive checksum. AC2=count is consumed.
memfs_snapshot_checksum:
        setz    3,
memfs_snapshot_checksum_loop:
        jumpe   2,memfs_snapshot_checksum_done
        add     3,(1)
        aoj     1,
        sojg    2,memfs_snapshot_checksum_loop
memfs_snapshot_checksum_done:
        move    1,3
        popj    17,

; int memfs_snapshot_mount(struct memfs *fs, unsigned int flags)
memfs_snapshot_mount:
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)
        move    010,1                  ; fs
        setzm   snapshot_span
        trnn    2,MEMFS_MOUNT_PERSIST
        jrst    memfs_snapshot_mount_ok
        skipn   backstore_enabled
        jrst    memfs_snapshot_mount_fail
        skipn   1,backstore_blocks
        jrst    memfs_snapshot_mount_fail

        move    011,MEMFS_POOL_WORDS(010)
        addi    011,SECTOR_WORDS-1
        lsh     011,-7                 ; ceil(pool words / 128)
        addi    011,1+SNAP_META_BLOCKS
        camge   011,backstore_blocks
        jrst    memfs_snapshot_mount_fail
        move    012,backstore_blocks
        sub     012,011                ; first reserved block

        move    1,backstore_blocks
        subi    1,1
        movei   2,1
        movei   3,fs_block_workspace
        pushj   17,backstore_read
        jumpn   1,memfs_snapshot_mount_fail

        move    1,fs_block_workspace
        came    1,[SNAP_MAGIC]
        jrst    memfs_snapshot_new
        move    1,fs_block_workspace+1
        caie    1,SNAP_VERSION
        jrst    memfs_snapshot_new
        move    1,fs_block_workspace+2
        came    1,012
        jrst    memfs_snapshot_new
        move    1,fs_block_workspace+3
        came    1,011
        jrst    memfs_snapshot_new
        move    1,fs_block_workspace+4
        came    1,MEMFS_POOL_WORDS(010)
        jrst    memfs_snapshot_new

        move    1,012
        movei   2,SNAP_META_BLOCKS
        move    3,MEMFS_NODES(010)
        pushj   17,backstore_read
        jumpn   1,memfs_snapshot_mount_fail
        move    1,MEMFS_NODES(010)
        movei   2,SNAP_META_WORDS
        pushj   17,memfs_snapshot_checksum
        came    1,fs_block_workspace+5
        jrst    memfs_snapshot_mount_fail

        setzm   MEMFS_USED_WORDS(010)
        movei   013,1
memfs_snapshot_restore_loop:
        caile   013,077
        jrst    memfs_snapshot_mount_commit
        move    014,013
        imuli   014,7
        add     014,MEMFS_NODES(010)
        move    1,NODE_META(014)
        trnn    1,1                    ; MEMFS_F_USED
        jrst    memfs_snapshot_restore_next
        hrrz    1,1
        lsh     1,-017                 ; type field starts at bit 15 decimal
        andi    1,7
        caie    1,VFS_TYPE_REG
        jrst    memfs_snapshot_restore_next
        hrrz    1,NODE_DATA(014)
        jumpe   1,memfs_snapshot_restore_next
        addm    1,MEMFS_USED_WORDS(010)
        hrrzs   NODE_DATA(014)         ; demand-load from persisted backing
memfs_snapshot_restore_next:
        aoja    013,memfs_snapshot_restore_loop

memfs_snapshot_new:
        move    015,backstore_blocks
        sub     015,backstore_blocks_used ; free blocks
        move    016,backstore_blocks
        caile   016,SECTOR_WORDS
        movei   016,SECTOR_WORDS
        camle   015,016                ; free <= process reserve
        jrst    memfs_snapshot_mount_fail
        move    1,015
        sub     1,016
        camle   011,1
        jrst    memfs_snapshot_mount_fail

        move    013,012
        move    014,012
        add     014,011                ; exclusive end
memfs_snapshot_bitmap_check:
        caml    013,014
        jrst    memfs_snapshot_bitmap_mark_start
        move    1,013
        idivi   1,044
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        tdne    3,(1)
        jrst    memfs_snapshot_mount_fail
        aoja    013,memfs_snapshot_bitmap_check

memfs_snapshot_bitmap_mark_start:
        move    013,012
memfs_snapshot_bitmap_mark:
        caml    013,014
        jrst    memfs_snapshot_bitmap_done
        move    1,013
        idivi   1,044
        add     1,backstore_bitmap
        movei   3,1
        lsh     3,0(2)
        iorm    3,(1)
        aoja    013,memfs_snapshot_bitmap_mark
memfs_snapshot_bitmap_done:
        addm    011,backstore_blocks_used

memfs_snapshot_mount_commit:
        hrlz    1,012                  ; first,,blocks also marks enabled
        ior     1,011
        movem   1,snapshot_span
memfs_snapshot_mount_ok:
        setz    1,
        jrst    memfs_snapshot_mount_return
memfs_snapshot_mount_fail:
        seto    1,
memfs_snapshot_mount_return:
        movei   0,010
        hrli    0,-6(17)
        blt     0,016
        sub     17,[7,,7]
        popj    17,

; int memfs_snapshot_shutdown(void)
memfs_snapshot_shutdown:
        add     17,[7,,7]
        movei   0,-6(17)
        hrli    0,010
        blt     0,(17)
        movei   010,memfs_mres_fs
        skipn   snapshot_span
        jrst    memfs_snapshot_shutdown_ok
        skipn   MEMFS_NODES(010)
        jrst    memfs_snapshot_shutdown_ok

        setzm   fs_block_workspace     ; invalidate commit header first
        move    1,backstore_blocks
        subi    1,1
        movei   2,1
        movei   3,fs_block_workspace
        pushj   17,backstore_write
        jumpn   1,memfs_snapshot_shutdown_fail

        hlrz    011,snapshot_span
        addi    011,SNAP_META_BLOCKS
        movei   012,1
memfs_snapshot_shutdown_loop:
        caile   012,077
        jrst    memfs_snapshot_shutdown_metadata
        move    013,012
        imuli   013,7
        add     013,MEMFS_NODES(010)
        move    1,NODE_META(013)
        trnn    1,1
        jrst    memfs_snapshot_shutdown_next
        hrrz    1,1
        lsh     1,-017                 ; type field starts at bit 15 decimal
        andi    1,7
        caie    1,VFS_TYPE_REG
        jrst    memfs_snapshot_shutdown_next
        hrrz    014,NODE_DATA(013)
        jumpe   014,memfs_snapshot_shutdown_next
        move    1,010
        move    2,012
        pushj   17,memfs_data_ensure
        jumpn   1,memfs_snapshot_shutdown_fail
        move    015,014
        addi    015,SECTOR_WORDS-1
        lsh     015,-7                 ; file backing blocks
        move    1,011
        add     1,015
        camge   1,backstore_blocks
        jrst    memfs_snapshot_shutdown_fail
        move    1,011
        move    2,015
        hlrz    3,NODE_DATA(013)
        pushj   17,backstore_write
        jumpn   1,memfs_snapshot_shutdown_fail
        hrlz    1,011
        ior     1,015
        move    2,MEMFS_POOL(010)
        add     2,012
        movem   1,(2)
        add     011,015
memfs_snapshot_shutdown_next:
        aoja    012,memfs_snapshot_shutdown_loop

memfs_snapshot_shutdown_metadata:
        hlrz    1,snapshot_span
        movei   2,SNAP_META_BLOCKS
        move    3,MEMFS_NODES(010)
        pushj   17,backstore_write
        jumpn   1,memfs_snapshot_shutdown_fail
        move    1,[SNAP_MAGIC]
        movem   1,fs_block_workspace
        movei   1,SNAP_VERSION
        movem   1,fs_block_workspace+1
        hlrz    1,snapshot_span
        movem   1,fs_block_workspace+2
        hrrz    1,snapshot_span
        movem   1,fs_block_workspace+3
        move    1,MEMFS_POOL_WORDS(010)
        movem   1,fs_block_workspace+4
        move    1,MEMFS_NODES(010)
        movei   2,SNAP_META_WORDS
        pushj   17,memfs_snapshot_checksum
        movem   1,fs_block_workspace+5
        move    1,backstore_blocks
        subi    1,1
        movei   2,1
        movei   3,fs_block_workspace
        pushj   17,backstore_write
        jumpn   1,memfs_snapshot_shutdown_fail
memfs_snapshot_shutdown_ok:
        setz    1,
        jrst    memfs_snapshot_shutdown_return
memfs_snapshot_shutdown_fail:
        seto    1,
memfs_snapshot_shutdown_return:
        movei   0,010
        hrli    0,-6(17)
        blt     0,016
        sub     17,[7,,7]
        popj    17,

        .bss
; Zero means volatile/disabled; otherwise first-block,,reserved-block-count.
snapshot_span:
        .block  1

