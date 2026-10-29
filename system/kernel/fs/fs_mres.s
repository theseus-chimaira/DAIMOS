; fs_mres.s -- fixed resident bridge to optional filesystem MRES.
        .text
        .globl fs_provider_reg_call
        .globl fs_memfs_service_jump
        .globl fs_dtfs_service_jump
        .globl fs_d6fs_service_jump
        .globl fs_tsfs_service_jump
        .globl fs_mres_no_service
        .globl proc_wait_event
        .globl proc_wakeup_event

        .globl blockset_runtime_service_jump
        .globl blockset_tail_blocks
        .globl blockset_direct_configure
        .globl device_range_configure
        .globl volume_tail_configure
        .globl blockset_direct_blocks
        .globl blockset_direct_tail
        .globl backing_region_blocks
        .globl device_region_map
        .globl volume_data_blocks
        .globl backing_region_count
        .globl dsk270_read_sector
        .globl dsk270_write_sector
        .globl drm236_read_block
        .globl drm236_write_block

blockset_runtime_service_jump:
        jrst    fs_mres_no_service

; Legacy configuration entry: AC3 nonzero requests VOLUME (BLOCKSET)
; mapping; AC3 zero selects a direct DEVICE range. Preserve its ABI.
blockset_direct_configure:
volume_tail_configure:
        move    5,1
        lsh     5,022
        ior     5,2
        movem   5,device_region_map
        movem   3,volume_data_blocks
        movem   4,backing_region_count
        popj    17,

; DEVICE: one physical unit, bounded contiguous range, no VOLUME mapper.
; AC1=unit selector, AC2=physical start, AC3=block count.
; Pass through the legacy state layout until all consumers are migrated.
device_range_configure:
        move    4,3
        setz    3,
        jrst    blockset_direct_configure

        .globl  backstore_read
backstore_read:
blockset_direct_tail_read:
        setz    4,
        jrst    blockset_direct_tail_io
        .globl  backstore_write
backstore_write:
blockset_direct_tail_write:
        movei   4,1

blockset_direct_tail_io:
        jumpe   2,kret_zero
        jumpe   3,kret_neg1
        jumpl   1,kret_neg1
        jumpl   2,kret_neg1
        move    6,backing_region_count
        jumpe   6,kret_neg1
        caml    1,6
        jrst    kret_neg1
        sub     6,1
        camle   2,6
        jrst    kret_neg1
        jumpe   4,blockset_direct_tail_account_read
        jrst    blockset_direct_tail_account_done
blockset_direct_tail_account_read:
blockset_direct_tail_account_done:
        add     17,kconst_6_6
        movei   0,-5(17)
        hrli    0,010
        blt     0,(17)
        move    010,3                  ; buffer
        move    011,1                  ; logical tail block
        move    012,2                  ; count
        move    013,4                  ; write flag

blockset_direct_tail_loop:
        skipn   volume_data_blocks
        jrst    device_range_translate
        ; A configured multi-member VOLUME requires its mapper.  Treat a
        ; missing/unloaded mapper as an I/O error; direct arithmetic would
        ; silently access the wrong physical sector on the first unit.
        hrrz    4,blockset_runtime_service_jump
        cain    4,fs_mres_no_service
        jrst    blockset_direct_tail_error
        move    1,011
        add     1,volume_data_blocks ; offset past filesystem data
        pushj   17,(4)
        jumpl   1,blockset_direct_tail_done
        jrst    device_block_transfer

blockset_direct_tail_error:
        seto    1,
        jrst    blockset_direct_tail_done

; Direct DEVICE path: selector/physical base come from the configured range.
device_range_translate:
        hlrz    1,device_region_map
        andi    1,07                     ; strip direct-device tag bits
        hrrz    2,device_region_map
        add     2,011

; Shared final hardware transfer, with no duplicate DSK/DRM dispatch.
device_block_transfer:
        move    3,010
        hlrz    0,device_region_map
        trne    0,0400000
        jrst    blockset_direct_tail_drm
        jumpe   013,blockset_direct_tail_read_one
        pushj   17,dsk270_write_sector
        jrst    blockset_direct_tail_after_one
blockset_direct_tail_read_one:
        pushj   17,dsk270_read_sector
        jrst    blockset_direct_tail_after_one
blockset_direct_tail_drm:
        jumpe   013,blockset_direct_tail_drm_read
        pushj   17,drm236_write_block
        jrst    blockset_direct_tail_after_one
blockset_direct_tail_drm_read:
        pushj   17,drm236_read_block
blockset_direct_tail_after_one:
        jumpe   1,blockset_direct_tail_after_ok
        jrst    blockset_direct_tail_done
blockset_direct_tail_after_ok:
        addi    010,0200
        aoj     011,
        sojn    012,blockset_direct_tail_loop
        setz    1,
blockset_direct_tail_done:
        movei   0,010
        hrli    0,-5(17)
        blt     0,013
        sub     17,kconst_6_6
        popj    17,
blockset_tail_blocks:
backing_region_blocks:
        move    1,backing_region_count
        popj    17,

; Register provider ABI used by the private PDP-6 UUO bridge:
;   AC6       FS_MRES_OP_*
;   AC7       provider number (4 MEMFS, 5 DTFS, 6 D6FS, 7 TSFS)
;   AC1..AC5 request a..e
;
; MINIT already maintains one movable register-dispatch service per provider.
; Use that binding directly instead of adding another permanent republished
; entry point.
fs_provider_reg_call:
        subi    7,4                    ; service-table index, 0..3
        jumpl   7,fs_mres_no_service
        caile   7,3
        jrst    fs_mres_no_service
        jumpn   7,fs_provider_serialized ; index zero: MEMFS never sleeps
        hrrz    7,fs_memfs_service_jump(7)
        jrst    (7)

; DTFS, TSFS and D6FS share fs_block_workspace and provider state across calls.
; A physical disk request may sleep, so another process can otherwise enter
; the provider while the first request is suspended and corrupt that state.
; Serialize those providers across the complete call.  The saved arguments
; live on the process-private kernel stack while proc_wait_event() switches.
fs_provider_serialized:
        ; Keep the original request registers on the process-private stack
        ; for the entire lock wait.  A resumed waiter may lose live scratch
        ; ACs while another process owns the provider.
        add     17,kconst_7_7
        movei   0,-6(17)
        hrli    0,1
        blt     0,(17)
fs_provider_lock_retry:
        skipn   fs_provider_ready
        jrst    fs_provider_lock_wait
        setzm   fs_provider_ready
        movei   0,1
        hrli    0,-6(17)
        blt     0,7
        sub     17,kconst_7_7
        hrrz    7,fs_memfs_service_jump(7)
        ; The missing-service trampoline already returns -1 normally.
        ; Invoke it under the same lease and use the common unlock path.
        pushj   17,(7)
        jrst    fs_provider_unlock

; The executive is not process-preemptible between the ready test and clear.
fs_provider_lock_wait:
        movei   1,fs_provider_ready
        pushj   17,proc_wait_event
        jrst    fs_provider_lock_retry

fs_provider_unlock:
        push    17,1
        setom   fs_provider_ready
        movei   1,fs_provider_ready
        pushj   17,proc_wakeup_event
        pop     17,1
        popj    17,
fs_memfs_service_jump:
        jrst    fs_mres_no_service
fs_dtfs_service_jump:
        jrst    fs_mres_no_service
fs_d6fs_service_jump:
        jrst    fs_mres_no_service
fs_tsfs_service_jump:
        jrst    fs_mres_no_service

; KCORE memory-pressure bridge for reclaimable filesystem memory.
; AC1=requested words, AC2=0 cheap/discardable cache, AC2=1 backed eviction.
; The clean BCACHE L1 is common KCORE state; backed MEMFS eviction remains in
; its independently movable MRES.
        .globl  fs_memory_reclaim
        .globl  bcache_reclaim
        .globl  memfs_reclaim_jump
fs_memory_reclaim:
        jumpn   2,fs_memory_reclaim_memfs
        jrst    bcache_reclaim
fs_memory_reclaim_memfs:
        hrrz    4,memfs_reclaim_jump
        cain    4,fs_mres_no_service
        jrst    kret_zero
        jrst    (4)
memfs_reclaim_jump:
        jrst    fs_mres_no_service

; Orderly-halt bridge.  Volatile/unmounted MEMFS returns immediately in MRES;
; a persistent instance commits its snapshot before the hardware HALT.
        .globl  fs_memfs_shutdown
        .globl  memfs_shutdown_jump
fs_memfs_shutdown:
        hrrz    4,memfs_shutdown_jump
        cain    4,fs_mres_no_service
        jrst    kret_zero
        jrst    (4)
memfs_shutdown_jump:
        jrst    fs_mres_no_service
fs_mres_no_service:
        jrst    kret_neg1

        .globl  fs_copy_words
; void fs_copy_words(src, dst, count)
; Shared forward word copy for resident filesystem data paths.
; AC1=src, AC2=dst, AC3=count.  AC4 is scratch.
fs_copy_words:
        jumpe   3,fs_copy_words_done
        move    4,2
        hrl     4,1
        add     2,3
        subi    2,1
        blt     4,(2)
fs_copy_words_done:
        popj    17,

        .globl  fs_move_words
; void fs_move_words(src, dst, count)
; Overlap-safe resident word move.  Forward/non-overlapping copies retain the
; BLT fast path; an upward overlapping move walks backwards.
; AC1=src, AC2=dst, AC3=count.  AC4 is scratch.
fs_move_words:
        jumpe   3,fs_move_words_done
        move    4,2
        sub     4,1                    ; delta = dst - src
        jumple  4,fs_copy_words       ; dst <= src: forward copy is safe
        sub     4,3
        jumpge  4,fs_copy_words       ; dst >= src + count: no overlap
        add     1,3
        subi    1,1
        add     2,3
        subi    2,1
fs_move_words_back:
        move    4,(1)
        movem   4,(2)
        subi    1,1
        subi    2,1
        sojg    3,fs_move_words_back
fs_move_words_done:
        popj    17,

        .globl  fs_zero_words
; void fs_zero_words(dst, count)
; Shared contiguous word clear.  AC1=dst, AC2=count, AC3 is scratch.
fs_zero_words:
        jumpe   2,fs_zero_words_done
        setzm   (1)
        soje    2,fs_zero_words_done
        move    3,1
        aoj     3,
        hrl     3,1
        add     1,2
        blt     3,(1)
fs_zero_words_done:
        popj    17,

        .globl  fs_zero_block_workspace
        .globl  bcache_workspace_invalidate
; Zero the shared 128-word filesystem transfer block without disturbing AC1.
fs_zero_block_workspace:
        push    17,1
        pushj   17,bcache_workspace_invalidate
        movei   1,fs_block_workspace
        movei   2,0200
        pushj   17,fs_zero_words
        pop     17,1
        popj    17,

        .globl  fs_mres_vector_dispatch
; Filesystem provider vectors pack two 18-bit target addresses per word.
; Operations are 1..16; a zero halfword denotes an unsupported operation.
; AC6 is scratch after target selection and AC0..AC5 remain untouched.
fs_mres_vector_target:
        jumple  6,fs_mres_vector_target_none
        caile   6,020
        jrst    fs_mres_vector_target_none
        subi    6,1                     ; zero-based operation
        trne    6,1
        jrst    fs_mres_vector_target_odd
        lsh     6,-1
        add     7,6
        hlrz    7,(7)
        popj    17,
fs_mres_vector_target_odd:
        lsh     6,-1
        add     7,6
        hrrz    7,(7)
        popj    17,
fs_mres_vector_target_none:
        setz    7,
        popj    17,

; Register provider dispatcher.  AC6 is the operation, AC7 the packed vector,
; and request a..e are already in AC1..AC5.  The fifth C argument is staged in
; one stack word.
fs_mres_vector_dispatch:
        pushj   17,fs_mres_vector_target
        jumpe   7,fs_mres_no_service
        add     17,kconst_1_1               ; reserve C arg 5
        movem   5,(17)
        pushj   17,(7)
        sub     17,kconst_1_1
        popj    17,

        .globl  fs_mres_context_vector_dispatch
; Context-register provider dispatcher.  AC0 is the provider context, AC6 the
; operation, AC7 the packed vector, and request a..e are in AC1..AC5.  Provider
; C ABI receives context as argument 1, a..c as 2..4 and d/e as stack args 5/6.
fs_mres_context_vector_dispatch:
        pushj   17,fs_mres_vector_target
        jumpe   7,fs_mres_no_service
        add     17,kconst_2_2
        movem   4,(17)                  ; arg 5 = request d
        movem   5,-1(17)                ; arg 6 = request e
        move    4,3
        move    3,2
        move    2,1
        move    1,0                     ; arg 1 = context
        pushj   17,(7)
        sub     17,kconst_2_2
        popj    17,

        .data
fs_provider_ready:
        .word   1

        .bss
        .globl  d6fs_active_reader
; Active D6FS dispatch pointer only.  Reader state itself is dynamic/mount-owned.
d6fs_active_reader:  .block 1
        .globl  d6fs_reader_slots_base
        .globl  dtfs_media_base
        .globl  tsfs_file_shape_base
        .globl  tsfs_extent_media_base
        .data
; KINIT publishes the MM slab reader table before the first root mount.
; Until then the boot reader is held by d6fs_active_reader; there is no
; mounted VFS volume and the slot table must not be indexed.
d6fs_reader_slots_base:
        .word   0
; All three mount-indexed tape tables are initially inaccessible. MINIT
; prepares only the shared directory cache, not mount metadata. KINIT
; publishes the three MM-slab bases before root selection and TSFS mount.
dtfs_media_base:
        .word   0
tsfs_file_shape_base:
        .word   0
tsfs_extent_media_base:
        .word   0
        .bss
; Existing exported names are aliases, not additional resident words.
blockset_direct_map:
device_region_map:       .block 1     ; selector,,physical start
blockset_direct_blocks:
volume_data_blocks:      .block 1     ; 0 means direct DEVICE mode
blockset_direct_tail:
backing_region_count:    .block 1     ; logical BACKING capacity
