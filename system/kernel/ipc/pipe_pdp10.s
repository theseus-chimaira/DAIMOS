; pipe_pdp10.s -- native-word DAIMOS pipes/FIFOs for PDP-6/PDP-10.
;
; IPC is deliberately representation-neutral.  Every queue element is one
; opaque 36-bit machine word.  SIXBIT/S6REC, terminal characters, paper-tape
; bytes, card columns, and application messages are interpreted outside this
; module.  This keeps the hot IPC path small and avoids conversion work for
; programs that merely forward data.
;
; struct pipe word offsets:
;   0 next_fifo, 1 fifo_node, 2 state, 3 refs,
;   4 read_event, 5 write_event, 6..015 eight native-word ring slots.
; state is head,,count in native 18-bit halves.  Both values are bounded to
; 0..010, so HLRZ/HRRZ replace bit-field extraction and shifts in the hot path.
; Tail is (head + count) & 7.  refs is readers,,writers in two 18-bit halves.
; Named FIFOs and anonymous pipes use this exact same representation.
;
; READ_WORDS/WRITE_WORDS transfer as much as is immediately available/free,
; then return a legal short count.  They sleep only when no progress is
; possible.  This avoids a scheduler round trip per word without making
; arbitrary multiword application records atomic.
        .text
        .globl  file_table
        .globl  file_new_fd
        .globl  mm_alloc
        .globl  mm_free
        .globl  proc_wait_event
        .globl  proc_wait_event_intr
        .globl  proc_wakeup_event
        .globl  proc_event_apply
        .globl  proc_current_slot

; Allocate and initialize a 016-word pipe object.  AC1 is fifo_node, zero for
; an anonymous pipe.  Payload words need no clearing: only queued slots are
; ever read and every enqueue overwrites a complete 36-bit slot.
pipe_alloc:
        push    17,010
        move    010,1
        push    17,[0]                  ; returned base
        movei   5,(17)
        push    17,5                    ; fifth arg: basep
        movei   1,016                   ; 14 words
        movei   2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,5                     ; PIPE_MM_OWNER
        setz    4,                      ; MM_ALLOC_LOW
        pushj   17,mm_alloc
        sub     17,[1,,1]
        jumpn   1,pipe_alloc_fail
        move    1,(17)
        jumpe   1,pipe_alloc_fail
        setzm   (1)
        movei   2,1(1)
        hrli    2,(1)
        blt     2,5(1)                  ; clear metadata words 0..5
        movem   010,1(1)
        jrst    pipe_alloc_done
pipe_alloc_fail:
        setz    1,
pipe_alloc_done:
        sub     17,[1,,1]
        pop     17,010
        popj    17,


; Signal a pipe event in AC1 and wake every waiter.  A nonzero event value is
; sufficient; using SETOM avoids materializing/storing a separate constant.
pipe_signal_event:
        setom   (1)
        jrst    proc_wakeup_event

; Arm and sleep interruptibly on a user-visible pipe/FIFO event in AC1.
; Executive code is non-preemptible between clearing the event and publishing
; the wait.  ALRM returns -1; internal kernel waits continue to use the
; noninterruptible proc_wait_event entry directly.
pipe_wait_event:
        setzm   (1)
        jrst    proc_wait_event_intr

; Find the active named-FIFO object for AC1 vnode; return zero if absent.
pipe_fifo_find:
        move    2,1
        move    3,pipe_fifo_head
pipe_fifo_find_loop:
        jumpe   3,pipe_fifo_find_none
        came    2,1(3)
        jrst    pipe_fifo_find_next
        move    1,3
        popj    17,
pipe_fifo_find_next:
        move    3,(3)
        jrst    pipe_fifo_find_loop
pipe_fifo_find_none:
        setz    1,
        popj    17,

; Remove AC1 object from the active FIFO list.  The object itself stays live.
        .globl  pipe_fifo_unlink_object
pipe_fifo_unlink_object:
        movei   2,pipe_fifo_head
        move    3,pipe_fifo_head
pipe_fifo_unlink_loop:
        jumpe   3,pipe_fifo_unlink_done
        came    3,1
        jrst    pipe_fifo_unlink_next
        move    4,(3)
        movem   4,(2)
        setzm   (3)
        popj    17,
pipe_fifo_unlink_next:
        move    2,3
        move    3,(3)
        jrst    pipe_fifo_unlink_loop
pipe_fifo_unlink_done:
        popj    17,

; kword_t pipe_create(void)
; Descriptor allocation is deliberately delegated to file_new_fd().  Besides
; being smaller than a private two-slot scan, this keeps pipe descriptors on
; the same metadata/lock-family path as every other newly opened descriptor.
        .globl  pipe_create
pipe_create:
        push    17,010
        push    17,011
        setz    1,
        pushj   17,pipe_alloc
        jumpe   1,pipe_create_fail
        move    010,1                   ; object base
        tlo     1,070001                ; PIPE_PROVIDER, PIPE_KIND_STREAM
        movei   2,1                     ; FILE_O_READ
        setz    3,                      ; not a directory
        pushj   17,file_new_fd
        jumpl   1,pipe_create_free
        move    011,1                   ; read fd

        move    1,010
        tlo     1,070001
        movei   2,2                     ; FILE_O_WRITE
        setz    3,
        pushj   17,file_new_fd
        jumpl   1,pipe_create_rollback

        move    4,[01000001]            ; one reader, one writer
        movem   4,3(010)
        hrl     1,011                   ; read fd,,write fd
        jrst    pipe_create_done
pipe_create_rollback:
        move    2,011
        lsh     2,1
        add     2,file_table
        setzm   (2)
pipe_create_free:
        move    1,010
        movei   2,3                     ; MM_TYPE_KERNEL_DYNAMIC
        movei   3,5                     ; PIPE_MM_OWNER
        pushj   17,mm_free
pipe_create_fail:
        seto    1,
pipe_create_done:
        pop     17,011
        pop     17,010
        popj    17,

; vnode_t pipe_fifo_open(vnode_t fifo_node, kword_t node_meta)
        .globl  pipe_fifo_open
pipe_fifo_open:
        push    17,010
        push    17,011
        move    7,1                     ; persistent FIFO vnode
        move    011,2                   ; requested descriptor metadata
        jumpe   7,pipe_fifo_open_bad
        jumpge  011,pipe_fifo_open_no_read
        jrst    pipe_fifo_open_access_ok
pipe_fifo_open_no_read:
        tlnn    011,0200000
        jrst    pipe_fifo_open_bad
pipe_fifo_open_access_ok:
        move    1,7
        pushj   17,pipe_fifo_find
        move    010,1
        jumpn   010,pipe_fifo_open_refs
        move    1,7
        pushj   17,pipe_alloc
        move    010,1
        jumpe   010,pipe_fifo_open_bad
        move    3,pipe_fifo_head
        movem   3,(010)
        movem   010,pipe_fifo_head

pipe_fifo_open_refs:
        jumpge  011,pipe_fifo_open_writer_ref
        movsi   3,1
        addm    3,3(010)
        movei   1,5(010)
        pushj   17,pipe_signal_event
pipe_fifo_open_writer_ref:
        tlnn    011,0200000
        jrst    pipe_fifo_open_rendezvous
        aos     3(010)
        movei   1,4(010)
        pushj   17,pipe_signal_event

pipe_fifo_open_rendezvous:
        jumpge  011,pipe_fifo_open_write_only
        tlne    011,0200000              ; O_RDWR never waits
        jrst    pipe_fifo_open_done
        hrrz    3,3(010)
        jumpn   3,pipe_fifo_open_done
        movei   1,4(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_fifo_open_intr
        jrst    pipe_fifo_open_done
pipe_fifo_open_write_only:
        hlrz    3,3(010)
        jumpn   3,pipe_fifo_open_done
        movei   1,5(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_fifo_open_intr
        jrst    pipe_fifo_open_done
pipe_fifo_open_intr:
        move    1,010
        move    2,011
        pushj   17,pipe_close_ref
        jrst    pipe_fifo_open_bad
pipe_fifo_open_done:
        move    1,010
        tlo     1,070001
        jrst    pipe_fifo_open_exit
pipe_fifo_open_bad:
        setz    1,
pipe_fifo_open_exit:
        pop     17,011
        pop     17,010
        popj    17,

        .globl  pipe_fifo_detach
pipe_fifo_detach:
        pushj   17,pipe_fifo_find
        jumpe   1,pipe_fifo_detach_done
        pushj   17,pipe_fifo_unlink_object
        setzm   1(1)
pipe_fifo_detach_done:
        popj    17,

        .globl  pipe_fifo_mount_busy
pipe_fifo_mount_busy:
        move    2,1
        move    3,pipe_fifo_head
pipe_fifo_mount_busy_loop:
        jumpe   3,pipe_fifo_mount_free
        ldb     4,[POINT 6,1(3),11]
        camn    4,2
        jrst    pipe_fifo_mount_yes
        move    3,(3)
        jrst    pipe_fifo_mount_busy_loop
pipe_fifo_mount_yes:
        movei   1,1
        popj    17,
pipe_fifo_mount_free:
        setz    1,
        popj    17,

; int pipe_read_words(vnode, ignored_offset, buf, nwords)
; VFS register ABI: AC1=node, AC3=destination, AC4=request.
        .globl  pipe_read_words
pipe_read_words:
        push    17,010
        push    17,011
        push    17,012
        hrrz    010,1
        move    011,3                   ; destination
        hrrz    012,4                   ; requested words
        jumpe   010,pipe_read_words_bad
        jumpe   011,pipe_read_words_bad
        jumpe   012,pipe_read_words_zero
pipe_read_words_retry:
        hrrz    4,2(010)                ; queued words
        jumpn   4,pipe_read_words_have
        hrrz    5,3(010)                ; writers
        jumpe   5,pipe_read_words_zero  ; EOF
        movei   1,4(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_read_words_bad
        jrst    pipe_read_words_retry
pipe_read_words_have:
        camle   4,012
        move    4,012                   ; transfer=min(count, request)
        move    012,4                   ; preserved return count
        hlrz    5,2(010)                ; head
        move    6,012
pipe_read_words_loop:
        movei   1,6(010)
        add     1,5
        move    1,(1)
        movem   1,(011)
        addi    011,1
        addi    5,1
        andi    5,7
        sojg    6,pipe_read_words_loop
        hrrz    4,2(010)
        sub     4,012
        hrl     5,5
        hrr     5,4
        movem   5,2(010)
        movei   1,5(010)
        pushj   17,pipe_signal_event
        move    1,012
        jrst    pipe_read_words_done
pipe_read_words_zero:
        setz    1,
        jrst    pipe_read_words_done
pipe_read_words_bad:
        seto    1,
pipe_read_words_done:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; int pipe_write_words(vnode, ignored_offset, buf, nwords)
; VFS register ABI: AC1=node, AC3=source, AC4=request.
        .globl  pipe_write_words
pipe_write_words:
        push    17,010
        push    17,011
        push    17,012
        hrrz    010,1
        move    011,3                   ; source
        hrrz    012,4                   ; requested words
        jumpe   010,pipe_write_words_bad
        jumpe   011,pipe_write_words_bad
        jumpe   012,pipe_write_words_zero
pipe_write_words_retry:
        hlrz    4,3(010)
        jumpe   4,pipe_write_words_broken
        hrrz    5,2(010)                ; queued words
        movei   6,010
        sub     6,5                     ; free slots
        jumpe   6,pipe_write_words_wait
        camle   6,012
        move    6,012                   ; transfer=min(free, request)
        move    012,6                   ; preserved return count
        hlrz    4,2(010)                ; head
        move    2,4
        add     2,5
        andi    2,7                     ; tail
        move    7,012
pipe_write_words_loop:
        move    1,(011)
        move    3,2
        addi    3,6(010)
        movem   1,(3)
        addi    011,1
        addi    2,1
        andi    2,7
        sojg    7,pipe_write_words_loop
        add     5,012
        hrl     4,4
        hrr     4,5
        movem   4,2(010)
        movei   1,4(010)
        pushj   17,pipe_signal_event
        move    1,012
        jrst    pipe_write_words_done
pipe_write_words_wait:
        movei   1,5(010)
        pushj   17,pipe_wait_event
        jumpl   1,pipe_write_words_bad
        jrst    pipe_write_words_retry
pipe_write_words_broken:
        move    1,proc_current_slot
        movei   2,7                     ; SYS_EVENT_PIPE
        pushj   17,proc_event_apply
pipe_write_words_bad:
        seto    1,
        jrst    pipe_write_words_done
pipe_write_words_zero:
        setz    1,
pipe_write_words_done:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

        .globl  pipe_add_ref
pipe_add_ref:
        move    6,1
        tlz     6,0707070               ; strip descriptor metadata
        lsh     6,-036
        caie    6,7                     ; PIPE_PROVIDER
        popj    17,
        hrrz    6,1
        jumpge  1,pipe_add_ref_write
        movsi   7,1
        addm    7,3(6)
pipe_add_ref_write:
        tlne    1,0200000
        aos     3(6)
        popj    17,

        .globl  pipe_add_refs
pipe_add_refs:
        jumpe   1,pipe_add_refs_done
        move    2,1
        movei   3,020
pipe_add_refs_loop:
        move    1,(2)
        pushj   17,pipe_add_ref
        addi    2,2
        sojg    3,pipe_add_refs_loop
pipe_add_refs_done:
        popj    17,

        .globl  pipe_close_ref
pipe_close_ref:
        push    17,010
        push    17,011
        hrrz    010,1
        jumpe   010,pipe_close_bad
        move    011,3(010)
        jumpge  2,pipe_close_writer
        hlrz    3,011
        jumpe   3,pipe_close_bad
        add     011,[-01000000]
pipe_close_writer:
        tlnn    2,0200000
        jrst    pipe_close_store
        hrrz    3,011
        jumpe   3,pipe_close_bad
        subi    011,1
pipe_close_store:
        movem   011,3(010)
        hlrz    3,011
        jumpn   3,pipe_close_writers
        movei   1,5(010)
        pushj   17,pipe_signal_event
pipe_close_writers:
        hrrz    3,011
        jumpn   3,pipe_close_live
        movei   1,4(010)
        pushj   17,pipe_signal_event
pipe_close_live:
        jumpn   011,pipe_close_ok
        skipn   1(010)
        jrst    pipe_close_free
        move    1,010
        pushj   17,pipe_fifo_unlink_object
pipe_close_free:
        move    1,010
        movei   2,3
        movei   3,5
        pushj   17,mm_free
        jumpn   1,pipe_close_bad
pipe_close_ok:
        setz    1,
        jrst    pipe_close_done
pipe_close_bad:
        seto    1,
pipe_close_done:
        pop     17,011
        pop     17,010
        popj    17,

        .bss
pipe_fifo_head:
        .block  1
