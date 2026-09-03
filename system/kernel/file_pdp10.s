; FILE runtime state is unconditional kernel state.  Keep it in KCORE BSS;
; RAMFS is an optional filesystem MRES and must not own FILE state.
        .bss
        .globl  file_table
file_table:
        .block  047                     ; 13 three-word struct file entries
        .globl  file_cwd
file_cwd:
        .block  1

        .text
; file_pdp10.s -- compact resident FILE/path primitives for PDP-6/PDP-10.
        .text

        .globl  file_path_char
; unsigned int file_path_char(path, pos)
; pos is bounded by FILE_PATH_MAX_CHARS, so signed IDIVI is sufficient and
; avoids constructing a 72-bit unsigned dividend for DIVI.
file_path_char:
        idivi   2,6
        move    4,3
        muli    4,6
        trne    4,1
        tloa    5,0400000
        tlz     5,0400000
        add     1,2
        move    1,1(1)
        move    4,5
        subi    4,036
        lsh     1,0(4)
        andi    1,077
        popj    17,

        .globl  file_path_setchar
; void file_path_setchar(path, pos, ch)
file_path_setchar:
        idivi   2,6
        move    5,3
        muli    5,6
        trne    5,1
        tloa    6,0400000
        tlz     6,0400000
        movei   4,036
        sub     4,6
        movei   5,077
        lsh     5,0(4)
        add     1,2
        andca   5,1(1)
        andi    3,077
        lsh     3,0(4)
        ior     5,3
        movem   5,1(1)
        popj    17,

        .globl  pdp10_ret_zero
        .globl  pdp10_ret_neg1

; int file_component(const kword_t *path, unsigned int *posp,
;     struct vfs_name *name)
;
; Packed paths and VFS names both use six 6-bit SIXBIT characters per word.
; Build byte pointers once and copy the component directly instead of
; repeatedly dividing, shifting, decoding to ASCII, and re-encoding.
        .globl  file_component
file_component:
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
; Clear the five-word vfs_name and make an output SIXBIT byte pointer.
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
        jrst    pdp10_ret_neg1

; int file_getcwd(kword_t *buf, unsigned int nwords)
;
; Construct cwd paths directly as packed SIXBIT.
        .globl  file_getcwd
file_getcwd:
        jumpe   1,file_getcwd_fail
        jumpge  2,file_getcwd_nwords_nonneg
        jrst    file_getcwd_nwords_ok    ; unsigned value with bit 35 set
file_getcwd_nwords_nonneg:
        cail    2,2
        jrst    file_getcwd_nwords_ok
        jrst    file_getcwd_fail
file_getcwd_nwords_ok:
        move    4,file_cwd
        jumpn   4,file_getcwd_have_node
        move    4,vfs_namespace_root
file_getcwd_have_node:
        move    5,4
        lsh     5,-036
        andi    5,077
        caige   5,4
        jrst    file_getcwd_pseudo_tail
        caile   5,6
        jrst    file_getcwd_pseudo_tail

; The 0121-word local area is one parent vnode followed by sixteen five-word
; vfs_name records.  AC15 is reused as output count after the upward walk.
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

        move    015,vfs_namespace_root

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
        move    1,010                   ; current vnode
        movei   2,(17)                   ; parent vnode
        move    3,4                      ; saved component name
        pushj   17,vfs_parent_name
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
        jrst    file_getcwd_pseudo
file_getcwd_fail:
        jrst    pdp10_ret_neg1


; struct file *file_find(int fd)
; File descriptors 3..15 map directly onto the 13 FILE records.
        .globl  file_find
file_find:
        caige   1,3
        jrst    file_find_fail
        caile   1,017
        jrst    file_find_fail
        subi    1,3
        imuli   1,3
        addi    1,file_table
        skipn   (1)
        jrst    file_find_fail
        popj    17,
file_find_fail:
        jrst    pdp10_ret_zero

; int file_new_fd(vnode_t node, unsigned int flags, int isdir)
; Table order is descriptor order, so the first free record is the lowest
; available descriptor and no resident fd bitmap or stored fd field is needed.
        .globl  file_new_fd
file_new_fd:
        move    0,2                    ; base metadata: flags
        andi    0,077
        jumpe   3,file_new_fd_nodir
        iori    0,0100                 ; FILE_META_DIR
file_new_fd_nodir:
        movei   4,file_table
        movei   5,3                    ; descriptor for current slot
        movei   6,015                  ; 13 slots
file_new_fd_scan:
        skipn   (4)
        jrst    file_new_fd_store
        addi    4,3
        addi    5,1
        sojg    6,file_new_fd_scan
file_new_fd_fail:
        jrst    pdp10_ret_neg1
file_new_fd_store:
        movem   1,(4)
        setzm   1(4)
        movem   0,2(4)
        move    1,5
        popj    17,

; int file_getcwd_pseudo(vnode_t node, kword_t *buf,
;     unsigned int nwords)
; Only the fixed DAIMOS 1.x synthetic directories can be current directories.
        .globl  file_getcwd_pseudo
file_getcwd_pseudo:
        move    4,2
        move    5,3
        jumpe   5,file_pseudo_zero_done
file_pseudo_zero:
        setzm   (4)
        addi    4,1
        sojg    5,file_pseudo_zero
file_pseudo_zero_done:
        camn    1,[020001000000]       ; /DEVICE
        jrst    file_pseudo_device
        camn    1,[020003000000]       ; /DEVICE/CTY0
        jrst    file_pseudo_cty
        camn    1,[030001000000]       ; /PROC
        jrst    file_pseudo_proc
        hlrz    4,1
        caie    4,030002               ; /PROC/{0,1}
        jrst    file_pseudo_fail
        hrrz    4,1
        cail    4,2
        jrst    file_pseudo_fail
        cail    3,3
        jrst    file_pseudo_proc_slot
        jrst    file_pseudo_fail
file_pseudo_device:
        cail    3,3
        jrst    file_pseudo_device_store
        jrst    file_pseudo_fail
file_pseudo_device_store:
        movei   4,7
        movem   4,(2)
        move    4,[0174445665143]      ; SIXBIT //DEVIC/
        movem   4,1(2)
        movsi   4,0450000              ; SIXBIT /E     /
        movem   4,2(2)
        jrst    pdp10_ret_zero
file_pseudo_cty:
        cail    3,4
        jrst    file_pseudo_cty_store
        jrst    file_pseudo_fail
file_pseudo_cty_store:
        movei   4,014
        movem   4,(2)
        move    4,[0174445665143]      ; SIXBIT //DEVIC/
        movem   4,1(2)
        move    4,[-0326034130660]     ; SIXBIT /E/CTY0/
        movem   4,2(2)
        jrst    pdp10_ret_zero
file_pseudo_proc:
        cail    3,2
        jrst    file_pseudo_proc_store
        jrst    file_pseudo_fail
file_pseudo_proc_store:
        movei   4,5
        movem   4,(2)
        move    4,[0176062574300]      ; SIXBIT //PROC /
        movem   4,1(2)
        jrst    pdp10_ret_zero
file_pseudo_proc_slot:
        movei   5,7
        movem   5,(2)
        move    5,[0176062574317]      ; SIXBIT //PROC//
        movem   5,1(2)
        lsh     4,036
        add     4,[0200000000000]
        movem   4,2(2)
        jrst    pdp10_ret_zero
file_pseudo_fail:
        jrst    pdp10_ret_neg1
