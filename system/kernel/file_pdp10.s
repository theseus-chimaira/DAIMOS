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
