; FILE runtime state lives in the RAMFS0 metadata prefix.
        .equ    file_v1_table,0601000
        .equ    file_v1_cwd,0601140
        .equ    file_v1_alias_cwd,0601141
        .globl  file_v1_table
        .globl  file_v1_cwd
        .globl  file_v1_alias_cwd

; file_pdp10.s -- compact resident FILE/path primitives for PDP-6/PDP-10.
        .text

; int file_v1_component(const kword_t *path, unsigned int *posp,
;     struct vfs_v1_name *name)
;
; Packed paths and VFS names both use six 6-bit SIXBIT characters per word.
; Build byte pointers once and copy the component directly instead of
; repeatedly dividing, shifting, decoding to ASCII, and re-encoding.
        .globl  file_v1_component
file_v1_component:
        jumpe   1,file_component_fail
        jumpe   2,file_component_fail
        jumpe   3,file_component_fail
        move    4,(1)           ; total path characters
        move    5,(2)           ; current character position
        caml    5,4             ; pos >= n
        jrst    file_component_empty

; Build a SIXBIT byte pointer to path[pos].  DIVI leaves quotient in AC6
; and remainder in AC7.  At most five bytes precede the requested position.
        move    7,5
        setz    6,
        divi    6,6
        add     6,1
        addi    6,1             ; path data starts at path[1]
        move    0,[POINT 6,0]
        hrr     0,6
        move    6,0
        move    0,7
file_component_seek:
        jumpe   0,file_component_skip
        ildb    7,6             ; discard bytes before path[pos]
        sojg    0,file_component_seek

; Skip leading separators.  SIXBIT '/' is 017.
file_component_skip:
        ildb    7,6
        caie    7,017
        jrst    file_component_start
        addi    5,1
        caml    5,4
        jrst    file_component_empty
        jrst    file_component_skip

file_component_start:
; Clear the five-word vfs_v1_name and make an output SIXBIT byte pointer.
        setzm   (3)
        movei   0,1(3)
        hrli    0,(3)
        blt     0,4(3)
        move    1,[POINT 6,0]
        movei   0,1(3)
        hrr     1,0
        movei   0,0             ; component character count

file_component_copy:
        cain    0,030           ; maximum is 24 characters
        jrst    file_component_fail
        idpb    7,1
        addi    0,1
        addi    5,1
        caml    5,4
        jrst    file_component_done
        ildb    7,6
        caie    7,017
        jrst    file_component_copy

; Consume the separator run before returning.  This leaves *posp at the
; next component (or n), so the C walker never needs a separate path-char
; decoder merely to skip '/'.
file_component_trailing:
        addi    5,1
        caml    5,4
        jrst    file_component_done
        ildb    7,6
        caie    7,017
        jrst    file_component_done
        jrst    file_component_trailing

file_component_done:
        movem   0,(3)
        movem   5,(2)
        movei   1,1
        popj    17,
file_component_empty:
        movem   5,(2)
        movei   1,0
        popj    17,
file_component_fail:
        seto    1,
        popj    17,

; int file_v1_getcwd(kword_t *buf, unsigned int nwords)
;
; Construct MEMFS cwd paths directly as packed SIXBIT.  DAIMOS 1.x has one
; live FILE context, so cwd and alias state are scalar rather than per-owner.
        .globl  file_v1_getcwd
file_v1_getcwd:
        skipn   file_v1_root
        jrst    file_getcwd_fail
        jumpe   1,file_getcwd_fail
        jumpge  2,file_getcwd_nwords_nonneg
        jrst    file_getcwd_nwords_ok    ; unsigned value with bit 35 set
file_getcwd_nwords_nonneg:
        cail    2,2
        jrst    file_getcwd_nwords_ok
        jrst    file_getcwd_fail
file_getcwd_nwords_ok:
        move    4,file_v1_cwd
        jumpn   4,file_getcwd_have_node
        move    4,[040001000000]         ; MEMFS root
file_getcwd_have_node:
        move    5,4
        lsh     5,-036
        andi    5,077
        caie    5,4
        jrst    file_getcwd_pseudo_tail

; The 0121-word local area is one parent vnode followed by sixteen five-word
; vfs_v1_name records.  AC15 is reused as output count after the upward walk.
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        add     17,[0121,,0121]
        move    010,4                    ; current node
        move    011,1                    ; output buffer
        move    012,2                    ; output words

; Scalar alias state is sufficient for the single live FILE context.
        movei   014,0
        skipn   file_v1_alias_cwd
        jrst    file_getcwd_stop_root
        movei   014,1
        move    015,file_v1_alias_node
        jrst    file_getcwd_stop_ready
file_getcwd_stop_root:
        move    015,[040001000000]
file_getcwd_stop_ready:

; Clear the complete supplied output record, preserving existing semantics.
        move    4,012
        move    5,011
file_getcwd_clear:
        setzm   (5)
        addi    5,1
        sojg    4,file_getcwd_clear

; Walk to the selected root, saving component names leaf-first.
        movei   013,0                    ; depth
file_getcwd_up:
        camn    010,015
        jrst    file_getcwd_up_done
        cail    013,020                  ; depth >= 16
        jrst    file_getcwd_local_fail
        move    4,013
        lsh     4,2
        add     4,013                    ; depth * 5
        movei   5,-0120(17)             ; parts[0]
        add     4,5
        move    1,file_v1_root
        move    2,010
        movei   3,(17)                   ; parent vnode
        pushj   17,memfs_v1_parent
        jumpn   1,file_getcwd_local_fail
        move    010,(17)
        addi    013,1
        jrst    file_getcwd_up

file_getcwd_up_done:
; Capacity is the number of SIXBIT characters in buf[1..nwords-1].
        move    5,012
        subi    5,1
        imuli   5,6
        jumpe   5,file_getcwd_local_fail
        move    6,[POINT 6,0]
        movei   4,1(011)
        hrr     6,4
        movei   015,0                    ; output character count

; Every cwd starts with '/'.
        movei   4,017
        idpb    4,6
        addi    015,1

; Alias paths start with /TEMP.
        jumpe   014,file_getcwd_components
        caige   5,5
        jrst    file_getcwd_local_fail
        movei   4,064                    ; T
        idpb    4,6
        movei   4,045                    ; E
        idpb    4,6
        movei   4,055                    ; M
        idpb    4,6
        movei   4,060                    ; P
        idpb    4,6
        addi    015,4

file_getcwd_components:
        jumpe   013,file_getcwd_store_len
        subi    013,1
        move    4,013
        lsh     4,2
        add     4,013
        movei   7,-0120(17)
        add     7,4                     ; AC7 -> component name
        move    3,(7)                   ; component chars

; Check room for the entire component and its separator before writing either.
        move    4,015
        cain    015,1
        jrst    file_getcwd_no_sep_need
        addi    4,1
file_getcwd_no_sep_need:
        add     4,3
        camle   4,5
        jrst    file_getcwd_local_fail
        cain    015,1
        jrst    file_getcwd_copy_name
        movei   4,017
        idpb    4,6
        addi    015,1

file_getcwd_copy_name:
        move    1,[POINT 6,0]
        movei   4,1(7)
        hrr     1,4
        jumpe   3,file_getcwd_components
file_getcwd_copy_loop:
        ildb    4,1
        idpb    4,6
        addi    015,1
        sojg    3,file_getcwd_copy_loop
        jrst    file_getcwd_components

file_getcwd_store_len:
        movem   015,(011)
        movei   1,0
        jrst    file_getcwd_return
file_getcwd_local_fail:
        seto    1,
file_getcwd_return:
        sub     17,[0121,,0121]
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; Non-MEMFS cwd formatting stays in the existing shared C helper.
file_getcwd_pseudo_tail:
        move    3,2
        move    2,1
        move    1,4
        jrst    file_v1_getcwd_pseudo
file_getcwd_fail:
        seto    1,
        popj    17,


; struct file_v1 *file_v1_find(int fd)
; Match the used/fd fields directly.  DAIMOS 1.x has one FILE owner, so file
; records no longer carry or compare an owner field.
        .globl  file_v1_find
file_v1_find:
        caige   1,3
        jrst    file_find_fail
        caile   1,017
        jrst    file_find_fail
        move    3,1
        lsh     3,010
        iori    3,1
        movei   4,file_v1_table
        movei   5,040
file_find_loop:
        move    6,2(4)
        xor     6,3
        tdnn    6,[037401]
        jrst    file_find_found
        addi    4,3
        sojg    5,file_find_loop
file_find_fail:
        movei   1,0
        popj    17,
file_find_found:
        move    1,4
        popj    17,

; int file_v1_new_fd(vnode_v1_t node, unsigned int flags, int isdir)
; Scan once, remembering the first free record and every descriptor already in
; use.  With one owner, every used descriptor belongs to the current context.
        .globl  file_v1_new_fd
file_v1_new_fd:
        move    0,2                    ; base metadata: flags
        andi    0,077
        lsh     0,2
        jumpe   3,file_new_fd_nodir
        iori    0,2
file_new_fd_nodir:
        iori    0,1                    ; FILE_V1_META_USED
        movei   2,0                    ; bitmap of used fd numbers
        movei   3,0                    ; first free table record
        movei   4,file_v1_table
        movei   5,040
file_new_fd_scan:
        move    6,2(4)
        trne    6,1
        jrst    file_new_fd_used
        jumpn   3,file_new_fd_next
        move    3,4
        jrst    file_new_fd_next
file_new_fd_used:
        lsh     6,-010
        andi    6,077
        movei   7,1
        lsh     7,0(6)
        ior     2,7
file_new_fd_next:
        addi    4,3
        sojg    5,file_new_fd_scan
        jumpe   3,file_new_fd_fail

        movei   5,3
        movei   6,010
file_new_fd_pick:
        tdnn    2,6
        jrst    file_new_fd_store
        lsh     6,1
        addi    5,1
        caile   5,017
        jrst    file_new_fd_fail
        jrst    file_new_fd_pick
file_new_fd_store:
        movem   1,(3)
        setzm   1(3)
        move    6,5
        lsh     6,010
        ior     6,0
        movem   6,2(3)
        move    1,5
        popj    17,
file_new_fd_fail:
        seto    1,
        popj    17,

; int file_v1_lookup_child(vnode_v1_t dir, const struct vfs_v1_name *name,
;     vnode_v1_t *nodep)
; Synthetic root entries compare their packed SIXBIT name directly.  All
; ordinary entries tail-call the owning provider with no stack frame.
        .globl  file_v1_lookup_child
file_v1_lookup_child:
        move    4,1
        lsh     4,-036
        andi    4,077
        caie    4,4
        jrst    file_lookup_child_not_memfs
        came    1,[040001000000]
        jrst    file_lookup_child_memfs_tail

        move    5,(2)                   ; name chars
        move    6,1(2)                  ; first packed SIXBIT word
        caie    5,6
        jrst    file_lookup_child_len4
        came    6,[-0333211263433]      ; DEVICE
        jrst    file_lookup_child_memfs_tail
        move    4,[020001000000]        ; DEVICEFS root
        jrst    file_lookup_child_store
file_lookup_child_len4:
        caie    5,4
        jrst    file_lookup_child_memfs_tail
        camn    6,[-0171520350000]      ; PROC
        jrst    file_lookup_child_proc
        skipn   4,file_v1_alias_node
        jrst    file_lookup_child_memfs_tail
        came    6,[-0133222200000]      ; TEMP
        jrst    file_lookup_child_memfs_tail
        jrst    file_lookup_child_store
file_lookup_child_proc:
        move    4,[030001000000]
file_lookup_child_store:
        movem   4,(3)
        movei   1,0
        popj    17,

file_lookup_child_memfs_tail:
        move    4,3
        move    3,2
        move    2,1
        move    1,file_v1_root
        jrst    memfs_v1_lookup
file_lookup_child_not_memfs:
        caie    4,2
        jrst    file_lookup_child_maybe_proc
        jrst    devicefs_v1_lookup
file_lookup_child_maybe_proc:
        caie    4,3
        jrst    file_lookup_child_fail
        jrst    procfs_v1_lookup
file_lookup_child_fail:
        seto    1,
        popj    17,

; int file_v1_readdir(int fd, struct vfs_v1_dirent *ent)
        .globl  file_v1_readdir
file_v1_readdir:
        push    17,010
        push    17,011
        push    17,012
        move    011,2                   ; ent
        pushj   17,file_v1_find
        move    010,1                   ; fp
        jumpe   1,file_readdir_fail
        jumpe   011,file_readdir_fail
        move    5,2(1)
        trnn    5,2                     ; FILE_V1_META_DIR
        jrst    file_readdir_fail
        move    2,(1)                   ; node
        move    4,2
        lsh     4,-036
        andi    4,077
        caie    4,4
        jrst    file_readdir_not_memfs

        move    1,file_v1_root
        move    3,1(010)
        move    4,011
        pushj   17,memfs_v1_readdir
        move    6,1                     ; rc
        jumpn   6,file_readdir_finish
        move    2,(010)
        came    2,[040001000000]
        jrst    file_readdir_finish

        ; Count ordinary MEMFS root entries to locate synthetic entries.
        ; AC1-AC7 are caller-saved; keep the ordinal in saved AC12 because
        ; memfs_v1_readdir clobbers AC7 while scanning node metadata.
        movei   012,0
file_readdir_base_loop:
        move    1,file_v1_root
        move    2,[040001000000]
        move    3,012
        move    4,011
        pushj   17,memfs_v1_readdir
        jumpg   1,file_readdir_base_more
        move    5,1(010)                ; requested visible offset
        camn    5,012
        jrst    file_readdir_synth_device
        addi    012,1
        camn    5,012
        jrst    file_readdir_synth_proc
        addi    012,1
        came    5,012
        jrst    file_readdir_eof
        skipn   file_v1_alias_node
        jrst    file_readdir_eof
        movei   5,4
        move    6,[-0133222200000]      ; TEMP
        jrst    file_readdir_synth_store
file_readdir_base_more:
        addi    012,1
        jrst    file_readdir_base_loop

file_readdir_synth_device:
        movei   5,6
        move    6,[-0333211263433]      ; DEVICE
        jrst    file_readdir_synth_store
file_readdir_synth_proc:
        movei   5,4
        move    6,[-0171520350000]      ; PROC
file_readdir_synth_store:
        movem   5,(011)
        movem   6,1(011)
        setzm   2(011)
        setzm   3(011)
        setzm   4(011)
        movei   5,1                     ; VFS_V1_TYPE_DIR
        movem   5,5(011)
        movei   6,1
        jrst    file_readdir_finish
file_readdir_eof:
        movei   6,0
        jrst    file_readdir_finish

file_readdir_not_memfs:
        caie    4,2
        jrst    file_readdir_maybe_proc
        move    1,2
        move    2,1(010)
        move    3,011
        pushj   17,devicefs_v1_readdir
        move    6,1
        jrst    file_readdir_finish
file_readdir_maybe_proc:
        caie    4,3
        jrst    file_readdir_fail
        move    1,2
        move    2,1(010)
        move    3,011
        pushj   17,procfs_v1_readdir
        move    6,1
file_readdir_finish:
        jumpg   6,file_readdir_advance
        move    1,6
        jrst    file_readdir_return
file_readdir_advance:
        aos     1(010)
        move    1,6
        jrst    file_readdir_return
file_readdir_fail:
        seto    1,
file_readdir_return:
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; unsigned int file_v1_used_slots(void)
; FILE records are three words and metadata is word two.  Count the fixed
; 32-entry table directly instead of emitting a general C array loop.
        .globl  file_v1_used_slots
file_v1_used_slots:
        movei   1,0
        movei   2,file_v1_table+2
        movei   3,040
file_used_slots_loop:
        move    4,(2)
        trne    4,1
        addi    1,1
        addi    2,3
        sojg    3,file_used_slots_loop
        popj    17,
