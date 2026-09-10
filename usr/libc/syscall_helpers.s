; syscall_helpers.s -- DAIMOS libc syscall helpers which are not
; one-instruction native ABI veneers.  These must remain callable and are
; deliberately excluded from dlink --daimos-uuo-relax.

        .text

        .globl dsys_write_nonets
        .globl dsys_mkdir
        .globl dsys_dtfs_check

dsys_write_nonets:
        move 4,[POINT 9,0,8]
        hrr 4,2
        move 2,4
        uuo 043,0(1)
        popj 17,

dsys_mkdir:
        movei 2,0777
        uuo 051,0(1)
        popj 17,

dsys_dtfs_check:
        iori 2,1
        uuo 065,0(1)
        popj 17,
