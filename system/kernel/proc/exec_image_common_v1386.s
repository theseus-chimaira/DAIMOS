; Staged common DXR image inspection/loading engine (not yet KCORE linked).
; Both initial EXEC and swapless PURE-prefix restoration are to use these
; leaves. Path lookup, privilege/RT checks, VM MM and scheduler publication
; remain the caller's responsibility.
;
; Shared compact four-word parsed image descriptor:
;   0: entry (LH18), initialized words (RH18)
;   1: DXR flags (LH18), BSS words (RH18)
;   2: PURE text words (LH18), header words (RH18, 2 or 3)
;   3: relocation map words (LH18), compressed payload words (RH18)
; No permanent storage is allocated. Caller owns descriptor lifetime.
;
; exec_image_inspect_v1386(vnode, file_words, descriptor):
;   Returns AC1=0 on a strictly valid DXR1/plain or DXR2/plain/compressed,
;   AC1=-1 on invalid bounds, header shape, short read, or corrupt metadata.
;   Does not write descriptor until all validations succeed.
;
; exec_image_apply_v1386(vnode, physical_image_destination,
;                        descriptor, prefix)
;   prefix=0: FULL uncompressed image, including initialized DATA.
;   prefix>0: PURE_PREFIX, in logical user words including origin 020.
;             Only [dest,dest+(prefix-020)) may be written.
;   AC1=0 success, -1 invalid/read error, -2 compressed PREFIX unsupported.
;   FULL compressed retains the existing D6LZ36 decoder semantics.
;   The prefix is already MM-reserved and immutable source protected.
;   There is no implicit relocation pass in the original EXEC loader.
        .text
        .globl exec_image_inspect_v1386
        .globl exec_image_apply_v1386
        .globl exec_image_prefix_backing_v1386
        .globl vfs_read_words
        .globl vfs_stat
        .globl d6lz36_decode_vfs
        .globl proc_swap_unpack_backing

        .equ EI_DXR_MAGIC_V1386,0447062
        .equ EI_DXR_TEXT_TAG_V1386,0647022
        .equ EI_MAX_IMAGE_V1386,0300000
        .equ EI_PURE_V1386,0200000
        .equ EI_COMP_V1386,0100000
        .equ EI_ORIGIN_V1386,020

exec_image_inspect_v1386:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        add     17,[03,,03]
        move    010,1                   ; vnode
        move    011,2                   ; file size, in words
        move    012,3                   ; caller descriptor
        jumpe   010,ei_bad_v1386
        jumpe   012,ei_bad_v1386
        caige   011,2
        jrst    ei_bad_v1386
        move    1,010
        setz    2,
        movei   3,-2(17)
        movei   4,2
        pushj   17,vfs_read_words
        caie    1,2
        jrst    ei_bad_v1386
        hlrz    1,-2(17)
        caie    1,EI_DXR_MAGIC_V1386
        jrst    ei_bad_v1386
        hrrz    014,-2(17)             ; executable entry offset
        hlrz    013,-1(17)             ; uncompressed image words
        hrrz    015,-1(17)             ; flags + BSS
        move    016,015
        andi    015,0700000             ; compressed/pure/RT-required
        andi    016,0077777             ; BSS (15 bits)
        jumpe   013,ei_bad_v1386
        caile   013,EI_MAX_IMAGE_V1386
        jrst    ei_bad_v1386
        caml    014,013
        jrst    ei_bad_v1386
        move    5,013
        addi    5,043
        idivi   5,044                   ; ceil(image words / 36)
        ; Scratch header[0] now reused: relmap in LH, compressed in RH.
        hrlz    6,5
        movem   6,-2(17)
        movei   7,2                     ; DXR1 unless shape requires DXR2
        move    5,011
        sub     5,013
        hlrz    6,-2(17)
        sub     5,6
        subi    5,2                     ; plain DXR1 => 0, DXR2 => 1
        trne    015,EI_COMP_V1386
        jrst    ei_compressed_v1386
        jumpe   5,ei_publish_v1386
        caie    5,1
        jrst    ei_bad_v1386
        movei   7,3
        jrst    ei_dxr2_v1386
ei_compressed_v1386:
        ; Compressed payload is before the reloc map and must be nonempty.
        move    5,011
        hlrz    6,-2(17)
        sub     5,6
        subi    5,3
        jumple  5,ei_bad_v1386
        hrrm    5,-2(17)
        movei   7,3
ei_dxr2_v1386:
        ; VFS is allowed to clobber AC0..AC7. Retain the selected DXR2
        ; header length in a dead stack word until the read completes.
        movem   7,-1(17)
        move    1,010
        movei   2,2
        movei   3,(17)
        movei   4,1
        pushj   17,vfs_read_words
        caie    1,1
        jrst    ei_bad_v1386
        hrrz    1,(17)
        caie    1,EI_DXR_TEXT_TAG_V1386
        jrst    ei_bad_v1386
        hlrz    6,(17)                 ; uncompressed text words
        camle   6,013
        jrst    ei_bad_v1386
        move    7,-1(17)
        jrst    ei_pack_v1386
ei_publish_v1386:
        setz    6,                      ; DXR1 has no TX2 text
ei_pack_v1386:
        hrlz    1,014
        hrr     1,013
        movem   1,(012)
        hrlz    1,015
        hrr     1,016
        movem   1,1(012)
        hrlz    1,6
        hrr     1,7
        movem   1,2(012)
        move    1,-2(17)
        movem   1,3(012)
        setz    1,
        jrst    ei_inspect_return_v1386
ei_bad_v1386:
        seto    1,
ei_inspect_return_v1386:
        sub     17,[03,,03]
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

exec_image_apply_v1386:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        move    010,1                   ; vnode
        move    011,2                   ; physical image destination
        move    012,3                   ; parsed descriptor
        move    013,4                   ; zero FULL, else PURE prefix
        jumpe   010,ei_apply_bad_v1386
        jumpe   011,ei_apply_bad_v1386
        jumpe   012,ei_apply_bad_v1386
        jumpe   013,ei_apply_full_v1386

        ; No RAW destination suffix, executable DATA, or saved user ACs
        ; may be overwritten. Match the exact complete 0200-word prefix.
        caig    013,EI_ORIGIN_V1386
        jrst    ei_apply_bad_v1386
        trne    013,0177
        jrst    ei_apply_bad_v1386
        hrrz    5,2(012)
        caie    5,3                     ; DXR2 required for PURE
        jrst    ei_apply_bad_v1386
        hlrz    6,2(012)
        jumpe   6,ei_apply_bad_v1386
        move    5,6
        addi    5,EI_ORIGIN_V1386
        andi    5,0777600
        came    5,013
        jrst    ei_apply_bad_v1386
        hlrz    5,1(012)               ; DXR flags
        trnn    5,EI_PURE_V1386
        jrst    ei_apply_bad_v1386
        trne    5,EI_COMP_V1386
        jrst    ei_apply_unsupported_v1386
        subi    013,EI_ORIGIN_V1386    ; exact plain prefix output words
        move    1,010
        movei   2,3                     ; DXR2 plain payload file offset
        move    3,011
        move    4,013
        pushj   17,vfs_read_words
        jrst    ei_apply_check_v1386

ei_apply_full_v1386:
        hlrz    5,1(012)
        trne    5,EI_COMP_V1386
        jrst    ei_apply_compressed_v1386
        move    1,010
        hrrz    2,2(012)               ; header words
        move    3,011
        hrrz    013,(012)              ; initialized image words
        move    4,013
        pushj   17,vfs_read_words
ei_apply_check_v1386:
        came    1,013
        jrst    ei_apply_bad_v1386
        setz    1,
        jrst    ei_apply_return_v1386
ei_apply_compressed_v1386:
        move    1,010
        hrrz    2,2(012)
        hrrz    3,3(012)               ; compressed payload words
        hrrz    4,(012)                ; unpacked initialized words
        hrl     4,4
        hrr     4,011
        pushj   17,d6lz36_decode_vfs
        jrst    ei_apply_return_v1386
ei_apply_unsupported_v1386:
        movni   1,2
        jrst    ei_apply_return_v1386
ei_apply_bad_v1386:
        seto    1,
ei_apply_return_v1386:
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,

; PURE restoration callback adapter for proc_pure_restore_v1384.
; (saved backing, physical APR base, stable u-area, omitted prefix).
; Decode the canonical mount/provider vnode with existing backing rules.
; Obtain the actual immutable executable size from VFS, call the SAME DXR
; inspector as EXEC, verify the bound text count, then apply PURE_PREFIX.
; The adapter never owns MM or publishes any process state. In particular,
; an unsupported compressed-prefix decode returns -2 without touching RAM.
exec_image_prefix_backing_v1386:
        push    17,010
        push    17,011
        push    17,012
        push    17,013
        push    17,014
        push    17,015
        push    17,016
        add     17,[013,,013]          ; 7 stat words, 4 metadata words
        move    010,1                  ; immutable backing record
        move    011,2                  ; physical APR base
        move    012,3                  ; stable u-area
        move    013,4                  ; dropped sector prefix
        jumpe   010,ei_bridge_bad_v1386
        jumpe   011,ei_bridge_bad_v1386
        jumpe   012,ei_bridge_bad_v1386
        jumpe   013,ei_bridge_bad_v1386
        move    1,010
        move    2,012
        pushj   17,proc_swap_unpack_backing
        move    014,1                  ; canonical vnode
        move    015,2                  ; expected PURE text words
        jumpe   014,ei_bridge_bad_v1386
        jumpe   015,ei_bridge_bad_v1386

        move    1,014
        movei   2,-010(17)             ; 7-word vfs_stat scratch
        pushj   17,vfs_stat
        jumpn   1,ei_bridge_bad_v1386
        move    2,-010(17)             ; vnode type
        caie    2,2                     ; VFS_TYPE_REG
        jrst    ei_bridge_bad_v1386
        move    1,014
        move    2,-005(17)             ; vfs_stat size_words, index 5
        movei   3,-003(17)             ; compact 4-word DXR meta
        pushj   17,exec_image_inspect_v1386
        jumpn   1,ei_bridge_bad_v1386
        hlrz    5,-1(17)              ; DXR2 text_words
        came    5,015
        jrst    ei_bridge_bad_v1386
        move    1,014
        move    2,011
        addi    2,EI_ORIGIN_V1386
        movei   3,-003(17)
        move    4,013
        pushj   17,exec_image_apply_v1386
        jrst    ei_bridge_return_v1386
ei_bridge_bad_v1386:
        seto    1,
ei_bridge_return_v1386:
        sub     17,[013,,013]
        pop     17,016
        pop     17,015
        pop     17,014
        pop     17,013
        pop     17,012
        pop     17,011
        pop     17,010
        popj    17,
