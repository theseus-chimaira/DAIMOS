; Compact PDP-6/PDP-10 executable loader.
; AC1=struct proc *, AC2=owner slot, AC3=counted SIXBIT path.
;
; Stack locals after the seven saved ACs and 013-word frame:
;   -012 vnode
;   -011..-003 struct vfs_stat (7 words; -006 is size_words)
;   -002..0 header words 0..2
; After validation, unused stat words are reused:
;   -010 relocation-map words
;   -007 header words (2 or 3)
;   -005 text words
;   -004 compressed payload words

        .text
        .globl  exec_load_process
        .globl  file_lookup_path
        .globl  file_check_access
        .globl  vfs_stat
        .globl  vfs_read_words
        .globl  vm_space_create
        .globl  vm_space_load_file
        .globl  vm_space_destroy
        .globl  proc_swap_attach
        .globl  d6lz36_decode_vfs

exec_load_process:
        push    17,10
        push    17,11
        push    17,12
        push    17,13
        push    17,14
        push    17,15
        push    17,16
        move    10,1                    ; proc
        move    11,2                    ; owner
        move    12,3                    ; path
        add     17,[013,,013]

        jumpe   10,exec_load_fail
        jumpe   12,exec_load_fail
        move    1,12
        movei   2,-012(17)              ; vnode
        pushj   17,file_lookup_path
        jumpn   1,exec_load_fail
        move    12,-012(17)             ; vnode stays in AC12

        move    1,12
        movei   2,-011(17)              ; struct vfs_stat
        pushj   17,vfs_stat
        jumpn   1,exec_load_fail
        move    0,-011(17)              ; type
        caie    0,2                     ; VFS_TYPE_REG
        jrst    exec_load_fail
        move    1,12
        movei   2,1                     ; execute access
        pushj   17,file_check_access
        jumpn   1,exec_load_fail
        move    0,-006(17)              ; size_words
        caige   0,2
        jrst    exec_load_fail

exec_load_read_header:
        move    1,12
        setz    2,
        movei   3,-002(17)
        movei   4,2
        pushj   17,vfs_read_words
        caie    1,2
        jrst    exec_load_fail
        hlrz    0,-002(17)
        caie    0,0447062               ; SIXBIT /DXR/
        jrst    exec_load_fail

        hrrz    14,-002(17)             ; entry
        hlrz    13,-001(17)             ; uncompressed image words
        hrrz    0,-001(17)
        move    15,0
        andi    15,0700000              ; compressed/pure/impure flags
        move    16,0
        andi    16,0077777              ; BSS words
        jumpe   13,exec_load_fail
        caile   13,036000
        jrst    exec_load_fail
        caile   16,020000
        jrst    exec_load_fail
        caml    14,13                   ; entry must be inside image
        jrst    exec_load_fail
        move    0,15
        andi    0,0600000
        cain    0,0600000               ; PURE and IMPURE are exclusive
        jrst    exec_load_fail

exec_load_header_shape:
        move    4,13
        addi    4,043
        idivi   4,044                   ; ceil(image_words / 36)
        movem   4,-010(17)              ; relocation-map words
        movei   0,2
        movem   0,-007(17)              ; header words
        setzm   -005(17)                ; text words
        setzm   -004(17)                ; compressed words
        trnn    15,0100000
        jrst    exec_load_plain_shape

        ; Compressed executables are always DXR2: the third word is required.
        ; The variable compressed payload lies before the normal reloc map.
        move    0,-006(17)              ; file size
        sub     0,-010(17)
        subi    0,3
        jumple  0,exec_load_fail
        movem   0,-004(17)
        jrst    exec_load_dxr2_header

exec_load_plain_shape:
        move    0,13
        add     0,-010(17)
        addi    0,2                     ; DXR1 expected words
        camn    0,-006(17)
        jrst    exec_load_create_vm
        addi    0,1                     ; DXR2 expected words
        came    0,-006(17)
        jrst    exec_load_fail

exec_load_dxr2_header:
        move    1,12
        movei   2,2
        movei   3,(17)
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    exec_load_fail
        hrrz    0,(17)
        caie    0,0647022               ; SIXBIT /TX2/
        jrst    exec_load_fail
        hlrz    0,(17)
        camle   0,13                    ; text_words <= image_words
        jrst    exec_load_fail
exec_load_dxr2_header_ok:
        movem   0,-005(17)
        movei   0,3
        movem   0,-007(17)

exec_load_create_vm:
        move    3,13
        add     3,16
        addi    3,02020                 ; user origin + user stack
        move    1,10
        move    2,11
        pushj   17,vm_space_create
        jumpn   1,exec_load_fail

        trnn    15,0100000
        jrst    exec_load_plain_image
        hrlz    4,13
        hrrz    0,1(10)                 ; VM physical base
        addi    0,020
        andi    0,0777777
        ior     4,0                     ; image_words,,destination
        move    1,12
        move    2,-007(17)
        move    3,-004(17)
        pushj   17,d6lz36_decode_vfs
        jumpn   1,exec_load_vm_fail
        jrst    exec_load_image_ok

exec_load_plain_image:
        move    3,-007(17)              ; load locals before fifth-arg push
        movei   4,020                   ; EXEC_USER_ORIGIN
        push    17,13                   ; fifth arg: image words
        move    1,10
        move    2,12
        pushj   17,vm_space_load_file
        sub     17,[1,,1]
        jumpn   1,exec_load_vm_fail

exec_load_image_ok:
        move    0,0(10)
        andi    0,0177400               ; preserve parent slot only
        move    1,14
        addi    1,020
        hrlz    1,1
        ior     0,1
        movem   0,0(10)
        movsi   0,024                   ; default nice bias
        tlz     0,0700000
        tlo     0,0100000               ; PROC_SIDL
        movem   0,2(10)

        setz    4,                      ; pure backing defaults false
        trnn    15,0200000
        jrst    exec_load_attach
        move    0,-007(17)
        caie    0,3
        jrst    exec_load_attach
        movei   4,1
exec_load_attach:
        move    1,11
        move    2,12
        move    3,-005(17)
        pushj   17,proc_swap_attach
        jumpn   1,exec_load_vm_fail
        jrst    exec_load_return

exec_load_vm_fail:
        move    1,10
        move    2,11
        pushj   17,vm_space_destroy
        jumpn   1,exec_load_fail
        hrrz    0,0(10)
        movem   0,0(10)                 ; clear entry LH after destroy
exec_load_fail:
        seto    1,
exec_load_return:
        sub     17,[013,,013]
        pop     17,16
        pop     17,15
        pop     17,14
        pop     17,13
        pop     17,12
        pop     17,11
        pop     17,10
        popj    17,
