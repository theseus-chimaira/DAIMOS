; Allocation-free D6LZ36 VFS streaming frontend for PDP-6/PDP-10.
;
; The complete token engine is the fixed low-core d6lz36_decode_core from
; system/stand/pdp6/common/decompressor.inc.  VFS only refills an eight-word
; source window and preserves the resumable decoder state around vfs_read_words.
;
; VFS entry:
;   AC1=vnode, AC2=file_offset, AC3=compressed_words,
;   AC4=dst_words,,dst_address.
;
; Eight input words are temporary stack storage, not permanent kernel RAM.
; Compared with the previous one-word frontend this also reduces VFS calls by
; up to 8x while keeping the memory/Stage1 decoder free of per-word calls.

        .text
        .globl d6lz36_decode_vfs
        .globl d6lz36_decode_core
        .globl vfs_read_words

        .equ D6LZ_VFS_WINDOW,010
        .equ D6LZ_VFS_LOCALS,012         ; vnode, offset, eight-word window

d6lz36_decode_vfs:
        ; The resumable core keeps control state in callee-saved AC10/AC11.
        ; AC12..AC15 keep the remaining VFS state across vfs_read_words().
        ; Reserve saved-AC slots plus locals in one step, then BLT AC10..AC15
        ; into the bottom six words.  This is five resident words smaller than
        ; six PUSH/POP pairs plus a separate local allocation.
        add     17,[020,,020]
        movei   0,-017(17)
        hrli    0,010
        blt     0,-012(17)
        movem   1,-011(17)              ; vnode
        movem   2,-010(17)              ; current file offset
        hlrz    13,4                    ; output words remaining
        hrrz    12,4                    ; current output address
        move    14,12                   ; output base
        move    15,3                    ; compressed words not yet read
        jumpe   13,d6lz_vfs_error
        jumpe   12,d6lz_vfs_error
        setz    11,                     ; zero => load a control word

d6lz_vfs_refill:
        jumpe   15,d6lz_vfs_error       ; core requested data past EOF
        move    1,-011(17)              ; vnode
        move    2,-010(17)              ; file offset
        movei   3,-07(17)               ; eight-word input window
        movei   4,D6LZ_VFS_WINDOW
        caige   15,D6LZ_VFS_WINDOW
        move    4,15                    ; final short window
        pushj   17,vfs_read_words
        jumpe   1,d6lz_vfs_error
        sub     15,1                    ; words still unread from file
        addm    1,-010(17)              ; advance file offset

        move    4,1                    ; source-window words returned
        movei   3,-07(17)
        pushj   17,d6lz36_decode_core
        jumpe   0,d6lz_vfs_success
        jumpl   0,d6lz_vfs_error
        jrst    d6lz_vfs_refill         ; +1 = NEED_INPUT

d6lz_vfs_success:
        jumpn   4,d6lz_vfs_error        ; exact compressed payload required
        jumpn   15,d6lz_vfs_error
        setz    1,
        jrst    d6lz_vfs_return

d6lz_vfs_error:
        seto    1,
d6lz_vfs_return:
        movei   0,-017(17)
        hrl     0,0
        hrri    0,010
        blt     0,015                   ; restore AC10..AC15
        sub     17,[020,,020]
        popj    17,
