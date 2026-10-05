/**
 * @file exec_load.s
 * @brief Compact resident DXR loader for the PDP-6/PDP-10 family.
 *
 * exec_load_process() validates a regular executable, checks execute access,
 * accepts DXR1 or DXR2 shape, optionally decompresses D6LZ36 payload directly
 * into the new VM, attaches pure-text swap backing, and publishes the initial
 * process entry/state only after the image is complete.  A failed load destroys
 * the staged VM and releases only an RT reservation acquired by this attempt.
 *
 * DXR2 is mandatory for compressed executables and carries the TX2 text-word
 * count used by pure backing.  RT_REQUIRED reserves the single proc_rt_owner;
 * an owner may replace its own RT image without dropping the reservation.
 *
 * Entry ABI for exec_load_process(): AC1=struct proc *, AC2=owner slot,
 * AC3=counted SIXBIT path.  AC10..AC16 are callee-saved as one contiguous BLT
 * block.  The combined 022-word frame contains those seven saved ACs plus the
 * following 013-word local area:
 *   -012       vnode
 *   -011..-003 struct vfs_stat (seven words; -006 is size_words)
 *   -002..0    DXR header words 0..2
 * After validation, dead stat fields are reused for:
 *   -010 relocation-map words
 *   -007 header words (2 or 3)
 *   -005 text words
 *   -004 compressed payload words
 *   -003 nonzero if this load acquired a new RT reservation
 */

        .text
        .globl  exec_load_process
        .globl  file_lookup_path
        .globl  file_check_access
        .globl  file_check_root
        .globl  vfs_stat
        .globl  vfs_read_words
        .globl  vm_space_create
        .globl  vm_space_load_file
        .globl  vm_space_destroy
        .globl  proc_swap_attach
        .globl  d6lz36_decode_vfs
        .globl  proc_rt_owner

        .equ    EXEC_DXR_MAGIC,0447062      ; SIXBIT /DXR/
        .equ    EXEC_DXR_TEXT_TAG,0647022   ; SIXBIT /TX2/
        .equ    EXEC_SCHED_SIDL_LH,0100024  ; SIDL + default nice bias


/**
 * @brief Validate one counted-SIXBIT startup record and return its word span.
 * @param AC1 Validated mapped record pointer.
 * @param AC2 Nonzero when an empty record is forbidden.
 * @return AC1 = count word + packed payload words, or zero if invalid.
 *
 * EXEC has already established that AC1 lies inside the mapped launch block;
 * this helper validates the 18-bit character count and the common DAIMOS
 * 102-character bounded-record contract. AC2/AC3 are caller-scratch.
 */
        .globl  sixbit_record_words
sixbit_record_words:
        move    3,(1)
        tlne    3,0777777              ; counted length must fit RH
        jrst    sixbit_record_bad
        jumpe   2,sixbit_record_length_ok
        jumpe   3,sixbit_record_bad    ; caller requires nonempty
sixbit_record_length_ok:
        caile   3,0146                 ; 102 characters maximum
        jrst    sixbit_record_bad
        move    1,3
        addi    1,013                  ; ceil(chars/6)+1 = (chars+11)/6
        idivi   1,6
        popj    17,
sixbit_record_bad:
        setz    1,
        popj    17,

/**
 * @brief Validate and load one DXR executable into an already allocated process.
 * @param AC1 Process descriptor to populate.
 * @param AC2 Process/VM owner slot.
 * @param AC3 Counted SIXBIT executable path.
 * @return AC1 = EXEC_LOAD_OK, EXEC_LOAD_RT_REQUIRED, or -1 on failure.
 *
 * The routine creates the VM only after validating file/header shape, and any
 * failure after VM creation destroys that staged VM before returning.  Entry PC
 * and runnable-state fields are published only after the payload has loaded.
 */
exec_load_process:
        ; Reserve locals plus the seven saved ACs in one step.  AC10..AC16
        ; are contiguous, so BLT is both smaller and faster than seven PUSHes.
        add     17,[022,,022]
        movei   0,-021(17)
        hrli    0,010
        blt     0,-013(17)
        move    10,1                    ; proc
        move    11,2                    ; owner
        move    12,3                    ; path
        setzm   -003(17)                ; no new RT reservation yet
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
        caie    0,EXEC_DXR_MAGIC
        jrst    exec_load_fail

        hrrz    14,-002(17)             ; entry
        hlrz    13,-001(17)             ; uncompressed image words
        hrrz    15,-001(17)
        move    16,15
        andi    15,0700000              ; compressed/pure/RT-required flags
        andi    16,0077777              ; BSS words
        jumpe   13,exec_load_fail
        caile   13,036000
        jrst    exec_load_fail
        caile   16,020000
        jrst    exec_load_fail
        caml    14,13                   ; entry must be inside image
        jrst    exec_load_fail
        trnn    15,0400000              ; RT_REQUIRED
        jrst    exec_load_header_shape
        pushj   17,file_check_root       ; RT-required admission is privileged
        jumpn   1,exec_load_fail
        skipn   0,proc_rt_owner
        jrst    exec_load_rt_claim
        came    0,11                    ; owner may replace its own image
        jrst    exec_load_fail
        jrst    exec_load_header_shape
exec_load_rt_claim:
        movem   11,proc_rt_owner
        setom   -003(17)

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
        move    0,-006(17)              ; actual file words
        sub     0,13
        sub     0,-010(17)
        subi    0,2                     ; 0=DXR1, 1=DXR2
        jumpe   0,exec_load_create_vm
        caie    0,1
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
        caie    0,EXEC_DXR_TEXT_TAG
        jrst    exec_load_fail
        hlrz    0,(17)
        camle   0,13                    ; text_words <= image_words
        jrst    exec_load_fail
exec_load_dxr2_header_ok:
        movem   0,-005(17)
        aos     -007(17)                ; header_words: DXR1 2 -> DXR2 3

exec_load_create_vm:
        move    3,13
        add     3,16
        addi    3,02020                 ; user origin + user stack
        move    1,10
        move    2,11
        pushj   17,vm_space_create
        jumpn   1,exec_load_nomem

        trnn    15,0100000
        jrst    exec_load_plain_image
        hrrz    4,1(10)                 ; VM physical base
        addi    4,020                    ; destination = base + user origin
        hrl     4,13                    ; image_words,,destination
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
        movsi   0,EXEC_SCHED_SIDL_LH
        movem   0,2(10)

        move    4,-007(17)              ; validated header_words is 2 or 3
        subi    4,2                     ; DXR2 => 1, DXR1 => 0
        trnn    15,0200000              ; non-pure images never keep backing
        setz    4,
exec_load_attach:
        move    1,11
        move    2,12
        move    3,-005(17)
        pushj   17,proc_swap_attach
        jumpn   1,exec_load_vm_fail
        movei   1,0
        trne    15,0400000              ; report RT_REQUIRED to caller
        movei   1,1
        jrst    exec_load_return

exec_load_vm_fail:
        move    1,10
        move    2,11
        pushj   17,vm_space_destroy
        jumpn   1,exec_load_fail
        hrrzs   0(10)                   ; clear entry LH after destroy
exec_load_fail:
        skipn   -003(17)                ; release only a reservation made here
        jrst    exec_load_fail_result
        camn    11,proc_rt_owner
        setzm   proc_rt_owner
exec_load_fail_result:
        seto    1,
        jrst    exec_load_return
exec_load_nomem:
        skipn   -003(17)                ; undo RT reservation before retry
        jrst    exec_load_nomem_result
        camn    11,proc_rt_owner
        setzm   proc_rt_owner
exec_load_nomem_result:
        move    1,[-2]                  ; EXEC_LOAD_NOMEM
exec_load_return:
        ; Restore the contiguous callee-saved range with one BLT.
        movei   0,010
        hrli    0,-021(17)
        blt     0,016
        sub     17,[022,,022]
        popj    17,
